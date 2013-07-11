#include "edititembase.h"

EditItemBase::EditItemBase()
    : id_(nextId())
{}

int EditItemBase::id() const { return id_; }

int EditItemBase::nextId()
{
    return nextId_++; // ### not thread safe; use a mutex for that
}

void EditItemBase::repaint()
{
    emit repaintNeeded();
}

int EditItemBase::nextId_ = 0;
