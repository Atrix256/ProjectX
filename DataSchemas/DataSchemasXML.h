/*==================================================================================================

DataSchemasXML.h

This expands the schemas defined in DataSchemas.h into xml parsing code

==================================================================================================*/
#pragma once

#include "DataSchemasStructs.h"
#include "External\tinyxml\tinyxml2.h"
#include <stdarg.h>

#define XMLLOGON 0
#define XMLERRORON 1

// logging functions
#if XMLLOGON
	inline void XMLLog(const char *format, ...)
	{
		printf("[XML] ");
		va_list args;
		va_start (args, format);
		vprintf (format, args);
		va_end (args);
		printf("\r\n");
	}
#else
	inline void XMLLog(const char *format, ...) { }
#endif

#if XMLERRORON
	inline void XMLError(const char *format, ...)
	{
		printf("[XML ERROR] ");
		va_list args;
		va_start (args, format);
		vprintf (format, args);
		va_end (args);
		printf("\r\n");
	}
#else
	inline void XMLError(const char *format, ...) { }
#endif

namespace DataSchemasXML {

	// LoadFromString() function
	template <typename T>
	inline bool LoadFromString (T &data, const char *stringData)
	{
		XMLError(__FUNCTION__" unhandled type");
		return false;
	}

	// Define the built in types that can be read from strings
	template <>
	inline bool LoadFromString<float> (float &data, const char *stringData)
	{
		if (sscanf(stringData, "%f", &data) == 1)
			return true;

		XMLError(__FUNCTION__" failure");
		return false;
	}

	template <>
	inline bool LoadFromString<bool> (bool &data, const char *stringData)
	{
		if (!stricmp(stringData,"true"))
		{
			data = true;
			return true;
		}
		else if (!stricmp(stringData,"false"))
		{
			data = false;
			return true;
		}

		int temp = 0;
		if (sscanf(stringData, "%i", &temp) == 1 && temp >= 0 && temp <= 1)
		{
			data = temp == 1;
			return true;
		}

		XMLError(__FUNCTION__" failure");
		return false;
	}

	template <>
	inline bool LoadFromString<std::string> (std::string &data, const char *stringData)
	{
		data = stringData;
		return true;
	}

	template <>
	inline bool LoadFromString<SData_Vec2> (SData_Vec2 &data, const char *stringData)
	{
		if (sscanf(stringData, "%f, %f", &data.m_x, &data.m_y) == 2)
			return true;

		XMLError(__FUNCTION__" failure");
		return false;
	}

	template <>
	inline bool LoadFromString<SData_Vec3> (SData_Vec3 &data, const char *stringData)
	{
		if (sscanf(stringData, "%f, %f, %f", &data.m_x, &data.m_y, &data.m_z) == 3)
			return true;

		XMLError(__FUNCTION__" failure");
		return false;
	}

	// Generalized Load(tinyxml2::XMLElement*) function
	template <typename T>
	inline bool Load (T &data, tinyxml2::XMLElement *node)
	{
		XMLLog(__FUNCTION__" starting");
		if (!node)
		{
			XMLError(__FUNCTION__" no node given");
			return false;
		}
		const tinyxml2::XMLAttribute *attr = node->FindAttribute("Value");
		if (attr)
		{
			if(LoadFromString(data, attr->Value())) {
				XMLLog(__FUNCTION__" loaded from 'Value' attribute.");
				return true;
			}
			XMLError(__FUNCTION__" failed to load from 'Value' attribute.");
			return false;
		} 

		XMLError(__FUNCTION__" failure");
		return false;
	}

// Load(tinyxml2::XMLElement*)
#define SchemaBegin(name) \
	template <> \
	inline bool Load <SData_##name> ( SData_##name &data, tinyxml2::XMLElement *node) \
	{ \
		unsigned int arrayIndex = 0; \
		XMLLog(__FUNCTION__" starting"); \
		if (!node) \
		{ \
			XMLError(__FUNCTION__" no node given"); \
			return false; \
		} \
		tinyxml2::XMLElement *childNode =  NULL; \
		const tinyxml2::XMLAttribute *attr = node->FindAttribute("Value"); \
		if (attr) \
		{ \
			if(LoadFromString(data, attr->Value())) {\
				XMLLog(__FUNCTION__" loaded from 'Value' attribute."); \
				return true; \
			} \
			XMLError(__FUNCTION__" failed to load from 'Value' attribute."); \
			return false; \
		} 
#define SchemaEnd \
	XMLLog(__FUNCTION__" succeeded"); \
	return true; }
#define Field(type, name, default) \
		XMLLog(__FUNCTION__" attempting to load field '%s'", #name); \
		attr = node->FindAttribute(#name); \
		if (attr) \
		{ \
			if(!LoadFromString(data.m_##name, attr->Value())) { \
				XMLError(__FUNCTION__" failed to load field '%s' from 'Value' attribute.", #name); \
				return false; \
			} \
		} \
		else \
		{ \
			childNode = node->FirstChildElement(#name); \
			if (childNode && !Load(data.m_##name, childNode)) {\
				XMLError(__FUNCTION__" failed to load field '%s' from childnode", #name); \
				return false; \
			} \
		}
#define Field_Schema(type, name, default) \
		XMLLog(__FUNCTION__" attempting to load schema field '%s'", #name); \
		if (default != NULL && !LoadFromString(data.m_##name, default)) { \
			XMLError(__FUNCTION__" failed to load schema field '%s' default value from string", #name); \
			return false; \
		} \
		attr = node->FindAttribute(#name); \
		if (attr) \
		{ \
			if(!LoadFromString(data.m_##name, attr->Value())) { \
				XMLError(__FUNCTION__" failed to load schema field '%s' from 'Value' attribute.", #name); \
				return false; \
			} \
		} \
		else \
		{ \
			childNode = node->FirstChildElement(#name); \
			if (childNode && !Load(data.m_##name, childNode)) {\
				XMLError(__FUNCTION__" failed to load schema field '%s' from childnode", #name); \
				return false; \
			} \
		}
#define Field_Schema_Array(type, name) \
	XMLLog(__FUNCTION__" attempting to load schema field array '%s'", #name); \
	childNode = node->FirstChildElement(#name); \
	arrayIndex = 0; \
	while(childNode) \
	{ \
		SData_##type dataItem; \
		if (!Load(dataItem, childNode)) {\
			XMLError(__FUNCTION__" failed to load a schema field array item for '%s'[%u]", #name, arrayIndex); \
			return false; \
		} \
		data.m_##name.push_back(dataItem); \
		childNode = childNode->NextSiblingElement(#name); \
		arrayIndex++;\
	}

#include "DataSchemas.h"
#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array

	template <typename T>
	inline bool Load (T &data, const char *fileName, const char *nodeName)
	{
		XMLLog(__FUNCTION__" %s", fileName);
		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(fileName) != tinyxml2::XML_NO_ERROR) {
			XMLError(__FUNCTION__" could not load xml file '%s'", fileName);
			return false;
		}
		tinyxml2::XMLElement *node = doc.FirstChildElement(nodeName);
		if (!node) {
			XMLError(__FUNCTION__" could not find node \"%s\"", nodeName);
			return false;
		}
		return Load(data, node);
	}
};
