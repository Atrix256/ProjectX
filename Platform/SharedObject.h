/*==================================================================================================

SharedObject.h

An object that can be passed to kernel code by memory as a cl_mem

==================================================================================================*/

#pragma once

#include "oclUtils.h"
#include <CL/cl_d3d10.h>
#include <CL/cl_d3d10_ext.h>
#include <CL/cl_ext.h>

#include "Platform/Assert.h"

template<typename T>
class CSharedObject
{
public:
	CSharedObject()
	{
		static_assert(sizeof(T) % 16 == 0, "CSharedObject type sizes must be multiples of 16");
		m_clData = NULL;
		m_clDataStale = true;
	}

	~CSharedObject()
	{
		Release();
	}

	void Release()
	{
		if (m_clData)
			clReleaseMemObject(m_clData);
		m_clData = NULL;
		m_clDataStale = true;
	}

	void EnsureCLMemExists (cl_context& context)
	{
		// allocate a new cl_mem object if needed
		if (!m_clData)
		{
			cl_int errorcode;
			m_clData = clCreateBuffer(
				context,
				CL_MEM_READ_WRITE,
				sizeof(T),
				NULL, 
				&errorcode
			);
			oclCheckErrorEX(errorcode, CL_SUCCESS, NULL);
			m_clDataStale = true;
			Assert_(m_clData != NULL);
		}
	}

	void ReadFromCLMem (cl_context& context, cl_command_queue& commandQueue)
	{
		EnsureCLMemExists(context);
		clEnqueueReadBuffer(commandQueue, m_clData, CL_TRUE, 0, sizeof(T), &m_object, 0, NULL, NULL);
		m_clDataStale = false;
	}

	cl_mem& GetAndWriteCLMem (cl_context& context, cl_command_queue& commandQueue)
	{
		EnsureCLMemExists(context);

		if (m_clDataStale)
		{
			clEnqueueWriteBuffer(commandQueue, m_clData, CL_FALSE, 0, sizeof(T), &m_object, 0, NULL, NULL);
			m_clDataStale = false;
		}

		return m_clData;
	}

	T &GetObject() { m_clDataStale = true; return m_object; }
	const T &GetObjectConst() { return m_object; }

private:
	T			m_object;
	cl_mem		m_clData;
	bool		m_clDataStale;
};