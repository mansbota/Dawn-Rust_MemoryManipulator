#pragma once

#include "aksInterface.h"

class BaseEntity
{
protected:
	uintptr_t baseAddress{};

	struct dataStruct {
		char data[2048];
	}data{};

public:
	BaseEntity() {}

	BaseEntity(uintptr_t gameObject, aksInterface* mem) :
		baseAddress{ mem->readPtrs(gameObject, { 0x30, 0x18, 0x28 }) },  //objectClass, entityPtr, baseEntity
		data{ mem->read<dataStruct>(baseAddress) }
	 {}
};

