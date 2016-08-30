#ifndef _copyable_H
#define _copyable_H

#include"msmuduo/stdafx.h"
NS_BEGIN
/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
};

NS_END

#endif
