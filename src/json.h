#pragma once

extern "C" {
#include "cJSON.h"
}

#include <string>
#include <cassert>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>

#ifndef STD
#define STD	std::
#endif

#ifndef ASSERT
#define ASSERT(cod) assert(cod)
#endif

class Json {
public:
	Json() : _cjson(nullptr), _isdel(true)
	{ }

	Json(const STD string & buffer) : Json()
	{
		FromBuffer(buffer);
	}

	Json(Json && json) : Json()
	{
		FromJson(STD move(json));
	}

	Json(const Json & json) : Json()
	{ 
		FromJson(json);
	}

	~Json()
	{
		if (_isdel)
		{
			Remove();
		}
	}

	Json & operator=(Json && json)
	{
		FromJson(STD move(json));
		return *this;
	}

	Json & operator=(const Json & json)
	{
		FromJson(json);
		return *this;
	}

	bool FromJson(Json && json)
	{
		Remove();
		_cjson = json._cjson;
		_isdel = json._isdel;
		json._cjson = nullptr;
		json._isdel = false;
		return nullptr != _cjson;
	}

	bool FromJson(const Json & json)
	{
		Remove();
		_cjson = Copy(json._cjson);
		_isdel = json._isdel;
		return nullptr != _cjson;
	}

	bool FromFile(const STD string & fname)
	{
		Remove();
		STD ifstream ifile(fname);
		STD string buffer;
		STD copy(STD istream_iterator<char>(ifile), 
				STD istream_iterator<char>(), 
				STD back_inserter(buffer));
		return FromBuffer(buffer);
	}

	bool FromBuffer(const STD string & buffer)
	{
		Remove();
		_cjson = cJSON_Parse(buffer.c_str());
		return nullptr != _cjson;
	}

	int GetType()
	{
		ASSERT(nullptr != _cjson);
		return _cjson->type;
	}

	template <class T>
	Json & InsertArray(T && val, int idx = INT_MAX)
	{
		ASSERT(nullptr != _cjson);
		InsertArrayImpl<T>(idx, STD forward<T>(val));
		return *this;
	}

	template <class T>
	Json & InsertHash(T && val, const STD string & key)
	{
		ASSERT(nullptr != _cjson);
		InsertHashImpl<T>(key, STD forward<T>(val));
		return *this;
	}

	void Remove()
	{
		cJSON_Delete(_cjson);
		_cjson = nullptr;
	}

	void Remove(int idx)
	{
		cJSON_DeleteItemFromArray(_cjson, idx);
	}

	void Remove(const STD string & key)
	{
		cJSON_DeleteItemFromObject(_cjson, key.c_str());
	}

	bool ToBool()
	{
		ASSERT(cJSON_True == _cjson->type || cJSON_False == _cjson->type);
		return 0 != _cjson->valueint;
	}

	int ToInt()
	{
		ASSERT(cJSON_Number == _cjson->type);
		return _cjson->valueint;
	}

	float ToFloat()
	{
		ASSERT(cJSON_Number == _cjson->type);
		return (float)_cjson->valuedouble;
	}

	double ToDouble()
	{
		ASSERT(cJSON_Number == _cjson->type);
		return _cjson->valuedouble;
	}

	STD string ToString()
	{
		ASSERT(cJSON_String == _cjson->type);
		return _cjson->valuestring;
	}

	Json GetValue(int idx)
	{
		return Json(cJSON_GetArrayItem(_cjson, idx), false);
	}

	Json GetValue(const STD string & key)
	{
		return Json(cJSON_GetObjectItem(_cjson, key.c_str()), false);
	}

	void ForEach(const STD function<bool (cJSON *)> & func)
	{
		for (auto ele = _cjson->child; ele != nullptr && func(ele); ele = ele->next);
	}

	STD string Print() const
	{
		return STD string(cJSON_Print(_cjson));
	}

private:
	Json(cJSON * cjson, bool isdel) : _cjson(cjson), _isdel(isdel)
	{ }

	//	number
	template <class T>
	void InsertArrayImpl(int idx, typename STD enable_if<STD is_arithmetic<T>::value && !STD is_same<bool, T>::value, T>::type val)
	{
		cJSON_InsertItemInArray(_cjson, idx, cJSON_CreateNumber(val));
	}

	template <class T>
	void InsertHashImpl(const STD string & key, typename STD enable_if<STD is_arithmetic<T>::value && !STD is_same<bool, T>::value, T>::type val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), cJSON_CreateNumber(val));
	}

	//	bool
	template <class T = bool>
	void InsertArrayImpl(int idx, const bool & val)
	{
		cJSON_InsertItemInArray(_cjson, idx, cJSON_CreateBool(val));
	}

	template <class T = bool>
	void InsertHashImpl(const STD string & key, const bool & val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), cJSON_CreateBool(val));
	}

	//	const char *
	template <class T = char>
	void InsertArrayImpl(int idx, const char * val)
	{
		cJSON_InsertItemInArray(_cjson, idx, cJSON_CreateString(val));
	}

	template <class T = char>
	void InsertHashImpl(const STD string & key, const char * val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), cJSON_CreateString(val));
	}

	//	string
	template <class T = STD string>
	void InsertArrayImpl(int idx, const STD string & val)
	{
		cJSON_InsertItemInArray(_cjson, idx, cJSON_CreateString(val.c_str()));
	}

	template <class T = STD string>
	void InsertHashImpl(const STD string & key, const STD string & val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), cJSON_CreateString(val.c_str()));
	}

	//	const Json &
	template <class T = Json>
	void InsertArrayImpl(int idx, const Json & val)
	{
		cJSON_InsertItemInArray(_cjson, idx, Copy(val._cjson));
	}

	template <class T = Json>
	void InsertHashImpl(const STD string & key, const Json & val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), Copy(val._cjson));
	}

	//	Json &&
	template <class T = Json>
	void InsertArrayImpl(int idx, Json && val)
	{
		cJSON_InsertItemInArray(_cjson, idx, val._cjson);
		val._cjson = nullptr;
	}

	template <class T = Json>
	void InsertHashImpl(const STD string & key, Json && val)
	{
		cJSON_AddItemToObject(_cjson, key.c_str(), val._cjson);
		val._cjson = nullptr;
	}

	cJSON * Copy(cJSON * parent) const
	{
		ASSERT(nullptr != parent);
		cJSON * cjson = nullptr;
		if (cJSON_Number == parent->type)
		{
			cjson = cJSON_CreateNumber(parent->valuedouble);
		}
		else if (cJSON_String == parent->type)
		{
			cjson = cJSON_CreateString(parent->valuestring);
		}
		else if (cJSON_False == parent->type)
		{
			cjson = cJSON_CreateFalse();
		}
		else if (cJSON_True == parent->type)
		{
			cjson = cJSON_CreateTrue();
		}
		else if (cJSON_Array == parent->type)
		{
			cjson = cJSON_CreateArray();
			for (auto ele = parent->child; ele != nullptr; ele = ele->next)
			{
				cJSON_AddItemToArray(cjson, Copy(ele));
			}
		}
		else if (cJSON_Object == parent->type)
		{
			cjson = cJSON_CreateObject();
			for (auto ele = parent->child; ele != nullptr; ele = ele->next)
			{
				cJSON_AddItemToObject(cjson, ele->string, Copy(ele));
			}
		}
		ASSERT(nullptr != cjson);
		return cjson;
	}

private:
	cJSON * _cjson;
	int _isdel;
};

inline STD ostream & operator<<(STD ostream & os, const Json & json)
{
	os << json.Print();
	return os;
}
