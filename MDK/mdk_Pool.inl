
namespace mdk
{

ObjectPoolNode::ObjectPoolNode (Allocator& allocator, size_t itemSize, size_t capacity)
    : allocator_ (allocator)
{
    m_assert (capacity >= 1 && "capacity must be at least 1.");

    _memory = allocator_.malloc (itemSize * capacity);
    if (_memory == nullptr)
        throw std::bad_alloc();

    _capacity = capacity;
    _nextNode = nullptr;
}

ObjectPoolNode::~ObjectPoolNode()
{
    allocator_.free (_memory);
}

template<typename TRAITS>
ObjectPool<TRAITS>::ObjectPool (size_t initialCapacity, size_t nodeMaxCapacity, Allocator& allocator)
    : _allocator (allocator)
    , _firstDeleted (nullptr)
    , _countInNode (0)
    , _nodeCapacity (initialCapacity)
    , _firstNode (_allocator, cItemSize, initialCapacity)
    , _nodeMaxCapacity (nodeMaxCapacity)
{
    m_assert (nodeMaxCapacity >= 1 && "nodeMaxCapacity must be at least 1.");

    _nodeMemory = _firstNode._memory;
    _lastNode = &_firstNode;
}

template<typename TRAITS>
ObjectPool<TRAITS>::~ObjectPool()
{
    SyncWrite sync (_syncHandle);

    ObjectPoolNode* node = _firstNode._nextNode;
    while (node)
    {
        ObjectPoolNode* nextNode = node->_nextNode;
        m_del (node);
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

    ObjectPoolNode* newNode = m_new<ObjectPoolNode> (_allocator, cItemSize, size);
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
