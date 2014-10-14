#include "mdk_Persistable.h"

namespace mdk
{

//==============================================================================
Persistable::Persistable()
{
}

Persistable::~Persistable()
{
    masterReference.clear();
}

DataStore Persistable::toDataStore (const Flags& flags)
{
    DataStore ds (classId());
    save (ds, flags);

    DataStore metads (Meta::id_Meta());
    saveMeta (metads, flags);

    if (metads.getNumProperties() > 0 || metads.getNumChildren() > 0)
        ds.addChild (metads, -1, nullptr);

    return ds;
}

void Persistable::fromDataStore (const DataStore& ds, const Flags& flags)
{
    if (ds.getType() != classId())
        return;

    load (ds, flags);
}

void Persistable::Meta::setReadOnly (DataStore& ds, const DataId& dataId)
{
    DataStore _ = ds.getOrCreateChildWithName (dataId, nullptr);
    saveValue (_, id_ReadOnly(), true);
}

void Persistable::Meta::setUIName (DataStore& ds, const DataId& dataId, const juce::String& uiname)
{
    DataStore _ = ds.getOrCreateChildWithName (dataId, nullptr);
    saveValue (_, id_UIName(), uiname);
}

void Persistable::Meta::setMinMaxInc (DataStore& ds, const DataId& dataId, double minVal, double maxVal, double incVal)
{
    DataStore _ = ds.getOrCreateChildWithName (dataId, nullptr);
    saveValue (_, id_Min(), minVal);
    saveValue (_, id_Max(), maxVal);
    saveValue (_, id_Inc(), incVal);
}

void Persistable::Meta::setMinMaxInc (DataStore& ds, const DataId& dataId, int minVal, int maxVal, int incVal)
{
    DataStore _ = ds.getOrCreateChildWithName (dataId, nullptr);
    saveValue (_, id_Min(), minVal);
    saveValue (_, id_Max(), maxVal);
    saveValue (_, id_Inc(), incVal);
}

void Persistable::Meta::setPathFilter (DataStore& ds, const DataId& dataId, const juce::String& filter, const juce::String& desc, const PathFlags& flags)
{
    DataStore _ = ds.getOrCreateChildWithName (dataId, nullptr);

    saveValue (_, id_PathFilter(), filter);
    saveValue (_, id_PathFlags(), flags.mask);

    if (desc.isNotEmpty())
        saveValue (_, id_Desc(), desc);
}

}
