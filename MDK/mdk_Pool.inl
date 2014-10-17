
namespace mdk
{

template<typename TRAITS>
ObjectPool<TRAITS>::_Node::_Node (Allocator& allocator, size_t capacity)
    : _allocator (allocator)
{
    jassert (capacity >= 1 && "capacity must be at least 1.");

    _memory = _allocator.malloc (cItemSize * capacity);
    if (_memory == nullptr)
        throw std::bad_alloc();

    _capacity = capacity;
    _nextNode = nullptr;
}

template<typename TRAITS>
ObjectPool<TRAITS>::_Node::~_Node()
{
    _allocator.free (_memory);
}

template<typename TRAITS>
ObjectPool<TRAITS>::ObjectPool (size_t initialCapacity, size_t nodeMaxCapacity, Allocator& allocator)
    : _allocator (allocator)
    , _firstDeleted (nullptr)
    , _countInNode (0)
    , _nodeCapacity (initialCapacity)
    , _firstNode (_allocator, initialCapacity)
    , _nodeMaxCapacity (nodeMaxCapacity)
{
    jassert (nodeMaxCapacity >= 1 && "nodeMaxCapacity must be at least 1.");

    _nodeMemory = _firstNode._memory;
    _lastNode = &_firstNode;
}

template<typename TRAITS>
ObjectPool<TRAITS>::~ObjectPool()
{
    SyncWrite sync (_syncHandle);

    _Node* node = _firstNode._nextNode;
    while (node)
    {
        _Node* nextNode = node->_nextNode;
        m_del (_allocator, node);
        node = nextNode;
    }
}

template<typename TRAITS>
void ObjectPool<TRAITS>::_allocateNewNode()
{
    size_t size = _countInNode;
    if (size >= _nodeMaxCapacity)
        size = _nodeMaxCapacity;
    else
    {
        size *= 2;

        jassert (size >= _countInNode && "size became too big.");

        if (size >= _nodeMaxCapacity)
            size = _nodeMaxCapacity;
    }

    _Node* newNode = m_new (_allocator, _Node) (_allocator, size);
    _lastNode->_nextNode = newNode;
    _lastNode = newNode;
    _nodeMemory = newNode->_memory;
    _countInNode = 0;
    _nodeCapacity = size;
}

template<typename TRAITS>
typename ObjectPool<TRAITS>::Object* ObjectPool<TRAITS>::allocate()
{
    SyncWrite sync (_syncHandle);

    if (_firstDeleted)
    {
        Object* result = _firstDeleted;
        _firstDeleted = * ((Object**)_firstDeleted);
        return result;
    }

    if (_countInNode >= _nodeCapacity)
        _allocateNewNode();

    char* address = (char*)_nodeMemory;
    address += _countInNode * cItemSize;

    Object* result = (Object*)address;
 
    _countInNode++;
    return result;
}

template<typename TRAITS>
bool ObjectPool<TRAITS>::isOwnerOf (Object* content) const
{
    SyncRead sync (_syncHandle);

    bool found = false;

    char* memCur = reinterpret_cast<char*> (content);

    _Node* node = &_firstNode;
    while (node && !found)
    {
        char* memBeg = static_cast<char*> (node->_memory);
        char* memEnd = memBeg;

        if (node == _lastNode)
            memEnd += ConstItemSize * _countInNode;
        else
            memEnd += ConstItemSize * node->_capacity;

        found = (memBeg <= memCur) && (memCur < memEnd);

        node = node->_nextNode;
    }

    return found;
}

template<typename TRAITS>
void ObjectPool<TRAITS>::release (Object* content)
{
    SyncWrite sync (_syncHandle);

    * ((Object**)content) = _firstDeleted;
    _firstDeleted = content;
}

template<typename TRAITS>
bool ObjectPool<TRAITS>::releaseSafe (Object* content)
{
    if (!isOwnerOf (content))
        return false;

    release (content);
    return true;
}

} // namespace
