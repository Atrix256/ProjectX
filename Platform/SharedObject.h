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

	cl_mem& GetAndUpdateCLMem (cl_context& context, cl_command_queue& commandQueue)
	{
		if (m_clDataStale)
		{
			// allocate a new cl_mem object if needed
			if (!m_clData)
			{
				cl_int errorcode;
				m_clData = clCreateBuffer(
					context,
					CL_MEM_READ_ONLY,
					sizeof(T),
					NULL, 
					&errorcode
				);
				oclCheckErrorEX(errorcode, CL_SUCCESS, NULL);
			}

			Assert_(m_clData != NULL);
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