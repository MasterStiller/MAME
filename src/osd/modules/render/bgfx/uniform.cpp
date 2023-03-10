// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniform.cpp - Shader uniform abstraction for BGFX layer
//
//============================================================

#include "uniform.h"

#include <cstring>
#include <utility>

bgfx_uniform::bgfx_uniform(std::string &&name, bgfx::UniformType::Enum type)
	: m_name(std::move(name))
	, m_type(type)
{
	m_handle = BGFX_INVALID_HANDLE;
	m_data_size = get_size_for_type(type);
	if (m_data_size > 0)
	{
		m_data = new uint8_t[m_data_size];
	}
}

bgfx_uniform::~bgfx_uniform()
{
	if (m_handle.idx != bgfx::kInvalidHandle)
	{
		bgfx::destroy(m_handle);
	}
	delete [] m_data;
}

void bgfx_uniform::create()
{
	m_handle = bgfx::createUniform(m_name.c_str(), m_type);
}

void bgfx_uniform::upload()
{
	if (m_type != bgfx::UniformType::Sampler)
	{
		bgfx::setUniform(m_handle, m_data);
	}
}

bgfx_uniform* bgfx_uniform::set(float* value)
{
	return set(value, get_size_for_type(bgfx::UniformType::Vec4));
}

bgfx_uniform* bgfx_uniform::set_int(int value)
{
	return set(&value, get_size_for_type(bgfx::UniformType::Sampler));
}

bgfx_uniform* bgfx_uniform::set_mat3(float* value)
{
	return set(value, get_size_for_type(bgfx::UniformType::Mat3));
}

bgfx_uniform* bgfx_uniform::set_mat4(float* value)
{
	return set(value, get_size_for_type(bgfx::UniformType::Mat4));
}

bgfx_uniform* bgfx_uniform::set(void* data, size_t size)
{
	int min_size = (size < m_data_size) ? size : m_data_size;
	memcpy(m_data, data, min_size);
	return this;
}

size_t bgfx_uniform::get_size_for_type(bgfx::UniformType::Enum type)
{
	switch (type)
	{
		case bgfx::UniformType::Vec4:
			return sizeof(float) * 4;

		case bgfx::UniformType::Sampler:
			return sizeof(int);

		case bgfx::UniformType::Mat3:
			return sizeof(float) * 3 * 3;

		case bgfx::UniformType::Mat4:
			return sizeof(float) * 4 * 4;

		default:
			return 0;
	}
}
