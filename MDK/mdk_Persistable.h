#ifndef MDK_PERSISTABLE_H_INCLUDED
#define MDK_PERSISTABLE_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_BitMask.h"

#include <modules/juce_data_structures/juce_data_structures.h>

namespace mdk
{
// http://www.juce.com/documentation/tutorials/valuetree-class
typedef juce::ValueTree DataStore;
typedef juce::Identifier DataId;

#define m_declare_class(name) const DataId& classId() override { static DataId _(#name); return _; }
#define m_declare_id(name) static const DataId& id_##name() { static DataId _(#name); return _; }
#define m_declare_attr(type, name) type name; m_declare_id(name);

/*! Provide persisting functions
*/
class Persistable
{
public:
    typedef juce::WeakReference<Persistable> Ref;

    enum Flag
    {
        flagSerialize,  // saving / loading from disk
        flagInspect,    // inspecting in runtime (e.g. editor)
    };

    typedef BitMask<Flag> Flags;

    Persistable();
    virtual ~Persistable();

    virtual const DataId& classId() = 0;
    virtual void save (DataStore& dst, const Flags& flags) = 0;
    virtual void saveMeta (DataStore& meta, const Flags& flags) = 0;
    virtual void load (const DataStore& src, const Flags& flags) = 0;

    DataStore toDataStore (const Flags& flags);
    void fromDataStore (const DataStore& ds, const Flags& flags);

    template<class T>
    static void clone (T& dst, T& src)
    {
        Flags flags;
        flags.turnOn (flagSerialize);

        DataStore ds = src.toDataStore (flags);
        dst.fromDataStore (ds, flags);
    }

public:
    // predefined id for meta data
    class Meta
    {
    public:
        m_declare_id (Meta);
        m_declare_id (ReadOnly);
        m_declare_id (UIName);
        m_declare_id (Min);
        m_declare_id (Max);
        m_declare_id (Inc);
        m_declare_id (PathFilter);
        m_declare_id (PathFlags);
        m_declare_id (Desc);

        enum PathFlag
        {
            pathIsDirectory, //! indicate the path filter should select directories instead of files.
            pathIsSaving, //! indicate the path filter should be opening rather than saving.
        };

        typedef BitMask<PathFlag, int> PathFlags;

        static void setReadOnly (DataStore& ds, const DataId& dataId);
        static void setUIName (DataStore& ds, const DataId& dataId, const juce::String& uiname);
        static void setMinMaxInc (DataStore& ds, const DataId& dataId, double minVal, double maxVal, double incVal);
        static void setMinMaxInc (DataStore& ds, const DataId& dataId, int minVal, int maxVal, int incVal);
        static void setPathFilter (DataStore& ds, const DataId& dataId, const juce::String& filter, const juce::String& desc, const PathFlags& flags);
    };

private:
    juce::WeakReference<Persistable>::Master masterReference;
    friend class juce::WeakReference<Persistable>;
};

template<typename TYPE> inline void saveValue (DataStore& dst, const DataId& dataId, const TYPE& value)
{
    dst.setProperty (dataId, juce::var (value), nullptr);
}

inline void saveValue (DataStore& dst, const DataId& dataId, const float& value)
{
    dst.setProperty (dataId, juce::var ((double)value), nullptr);
}

template<typename TYPE> inline void saveArray (DataStore& dst, const DataId& dataId, const juce::Array<TYPE>& values)
{
    juce::var copied;
    const int cSize = values.size();
    for (int it = 0; it < cSize; ++it)
    {
        copied.append (juce::var (values.getReference (it)));
    }
    dst.setProperty (dataId, copied, nullptr);
}

template<typename TYPE> inline bool loadValue (const DataStore& src, const DataId& dataId, TYPE& value)
{
    // check for existance
    if (!src.hasProperty (dataId))
        return false;

    const juce::var& prop = src.getProperty (dataId);

    // check for type mis-match
    juce::var refValue (value);
    if (!prop.hasSameTypeAs (refValue))
        return false;

    value = TYPE (prop);

    return true;
}

inline bool loadValue (const DataStore& src, const DataId& dataId, float& value)
{
    double value64;
    if (!loadValue (src, dataId, value64))
        return false;

    value = (float)value64;
    return true;
}

inline bool loadValue (const DataStore& src, const DataId& dataId, juce::String& value)
{
    // check for existance
    if (!src.hasProperty (dataId))
        return false;

    const juce::var& prop = src.getProperty (dataId);

    // check for type mis-match
    juce::var refValue (value);
    if (!prop.hasSameTypeAs (refValue))
        return false;

    value = prop.toString();

    return true;
}

template<typename TYPE> inline bool loadArray (const DataStore& src, const DataId& dataId, juce::Array<TYPE>& values)
{
    // check for existance
    if (!src.hasProperty (dataId))
        return false;

    const juce::var& prop = src.getProperty (dataId);

    // array checking
    if (!prop.isArray())
    {
        TYPE value;
        if (loadValue (src, dataId, value))
            values.add (value);
    }

    const juce::Array<juce::var>* arr = prop.getArray();
    if (arr->size() == 0)
        return true;

    //juce::var refValue (TYPE());
    //if (!arr->getReference(0).hasSameTypeAs (refValue))
    //    return false;

    const int cSize = arr->size();
    for (int it = 0; it < cSize; ++it)
    {
        values.add ((TYPE)arr->getReference (it));
    }

    return true;
}

}

#endif  // MDK_PERSISTABLE_H_INCLUDED
