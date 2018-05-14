// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

class ScriptModule;
class KBEDATATYPE_BASE;

/*
	EntityDef模块
	管理了所有的实体定义的描述以及所有的数据类型描述
*/
class  EntityDef
{
public:
	

	// 所有的数据类型
	static TMap<std::string, uint16> datatype2id;
	static TMap<std::string, KBEDATATYPE_BASE*> datatypes;
	static TMap<uint16, KBEDATATYPE_BASE*> id2datatypes;

	static TMap<std::string, int32> entityclass;

	static TMap<std::string, ScriptModule*> moduledefs;
	static TMap<uint16, ScriptModule*> idmoduledefs;

	static void initialize();

	static void clear();
	static void initDataType();
	static void bindMessageDataType();
};
