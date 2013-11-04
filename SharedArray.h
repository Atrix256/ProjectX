#pragma once

template<typename T>
class CSharedArray
{
public:
	CSharedArray()
	{
		m_clData = NULL;
		m_data = NULL;
		m_dataSize = 0;
		m_allocatedSize = 0;
		m_clDataStale = true;
	}

	~CSharedArray()
	{
		Release();
	}

	void Release()
	{
		if (m_clData)
			clReleaseMemObject(m_clData);
		m_clData = NULL;
		delete m_data;
		m_data = NULL;
		m_dataSize = 0;
		m_allocatedSize = 0;
		m_clDataStale = true;
	}

	unsigned int SizeInBytes () const { return sizeof(T) * m_dataSize; }
	unsigned int Count () const { return m_dataSize; }

	cl_mem& GetAndUpdateMem (cl_command_queue& commandQueue)
	{
		if (m_clDataStale)
		{
			clEnqueueWriteBuffer(commandQueue, m_clData, CL_FALSE, 0, SizeInBytes(), m_data, 0, NULL, NULL);
			m_clDataStale = false;
		}
		return m_clData;
	}

	T& operator[] (unsigned int index)
	{
		return m_data[index];
	}

	T& AddOne (cl_context& context)
	{
		unsigned int newIndex = Count();
		Resize(newIndex + 1, context);
		return (*this)[newIndex];
	}

	void Clear (cl_context& context)
	{
		Resize(0, context);
	}

	void Resize (unsigned int newSize, cl_context& context)
	{
		// our data is stale when we resize
		m_clDataStale = true;

		// if we can just change the size without reallocating and copying
		// go for it
		if (newSize <= m_allocatedSize)
		{
			m_dataSize = newSize;
			return;
		}

		// allocate a new buffer and copy the existing data over
		T* newData = new T[newSize];
		if (m_dataSize > 0 && newSize > 0)
			memcpy(newData, m_data, min(m_dataSize, newSize) * sizeof(T));

		// free the old data and set the pointer to the new data
		delete[] m_data;
		m_data = newData;

		// set the data size and allocated data size
		m_allocatedSize = newSize;
		m_dataSize = newSize;

		if (m_clData)
			clReleaseMemObject(m_clData);

		cl_int errorcode;
		m_clData = clCreateBuffer(
			context,
			CL_MEM_READ_ONLY,
			SizeInBytes(),
			NULL, 
			&errorcode
		);
		oclCheckErrorEX(errorcode, CL_SUCCESS, NULL);
	}

private:
	T			*m_data;
	unsigned int m_dataSize;
	unsigned int m_allocatedSize;
	cl_mem		 m_clData;
	bool		 m_clDataStale;
};