﻿/* Copyright 2024 The Kingsoft's modern-string Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#pragma once

#include "ks_string_view.h"
#include "ks_basic_pointer_iterator.h"
#include "ks_basic_string_allocator.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <ostream>

template <class ELEM>
class ks_basic_mutable_string;
template <class ELEM>
class ks_basic_immutable_string;


template <class ELEM>
class MODERN_STRING_API ks_basic_xmutable_string_base {
	static_assert(std::is_trivial_v<ELEM> && std::is_standard_layout_v<ELEM>, "ELEM must be pod type");

public:
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using value_type = ELEM;
	using reference = const ELEM&;
	using const_reference = const ELEM&;
	using pointer = const ELEM*;
	using const_pointer = const ELEM*;

	using iterator = ks_basic_pointer_iterator<const ELEM>;
	using const_iterator = ks_basic_pointer_iterator<const ELEM>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_t npos = size_t(-1);

public:
	//def ctor
	ks_basic_xmutable_string_base() 
		: ks_basic_xmutable_string_base(__raw_ctor::v) {
	}

	//copy & move ctor
	ks_basic_xmutable_string_base(const ks_basic_xmutable_string_base& other) {
		if (other.is_sso_mode()) {
			*_my_sso_ptr() = *other._my_sso_ptr();
		}
		else {
			*_my_ref_ptr() = *other._my_ref_ptr();
			ks_basic_string_allocator<ELEM>::_refcountful_addref(_my_ref_ptr()->alloc_addr());
		}
	}

	ks_basic_xmutable_string_base(ks_basic_xmutable_string_base&& other) noexcept {
		if (other.is_sso_mode()) 
			*_my_sso_ptr() = *other._my_sso_ptr();
		else 
			*_my_ref_ptr() = *other._my_ref_ptr();
		::new (&other) ks_basic_xmutable_string_base{};
	}

	ks_basic_xmutable_string_base& operator=(const ks_basic_xmutable_string_base& other) {
		if (this != &other) {
			if (other.is_sso_mode()) {
				this->~ks_basic_xmutable_string_base();
				*_my_sso_ptr() = *other._my_sso_ptr();
			}
			else {
				if (_my_ref_ptr()->alloc_addr() == other._my_ref_ptr()->alloc_addr()) {
					*_my_ref_ptr() = *other._my_ref_ptr();
				}
				else {
					this->~ks_basic_xmutable_string_base();
					*_my_ref_ptr() = *other._my_ref_ptr();
					ks_basic_string_allocator<ELEM>::_refcountful_addref(_my_ref_ptr()->alloc_addr());
				}
			}
		}
		return *this;
	}

	ks_basic_xmutable_string_base& operator=(ks_basic_xmutable_string_base&& other) noexcept {
		ASSERT(this != &other);
		this->~ks_basic_xmutable_string_base();
		if (other.is_sso_mode()) 
			*_my_sso_ptr() = *other._my_sso_ptr();
		else 
			*_my_ref_ptr() = *other._my_ref_ptr();
		::new (&other) ks_basic_xmutable_string_base(__raw_ctor::v);
		return *this;
	}

	//dtor
	~ks_basic_xmutable_string_base() {
		if (this->is_ref_mode()) {
			ks_basic_string_allocator<ELEM>::_refcountful_release(_my_ref_ptr()->alloc_addr());
		}
	}

protected:
	//raw ctor
	enum class __raw_ctor { v };
	explicit ks_basic_xmutable_string_base(__raw_ctor) noexcept {
		if (sizeof(_SSO_STRUCT) / 8 != 0)
			std::fill_n((uint64_t*)(_my_sso_ptr()), sizeof(_SSO_STRUCT) / 8, 0);
		if (sizeof(_SSO_STRUCT) % 8 != 0)
			std::fill_n((uint8_t*)(_my_sso_ptr()) + sizeof(_SSO_STRUCT) - sizeof(_SSO_STRUCT) % 8, sizeof(_SSO_STRUCT) % 8, 0);
	}

	//explicit ctor
	explicit ks_basic_xmutable_string_base(const ELEM* p) : ks_basic_xmutable_string_base(__to_basic_string_view(p)) {}
	explicit ks_basic_xmutable_string_base(const ELEM* p, size_t count) : ks_basic_xmutable_string_base(__to_basic_string_view(p, count)) {}

	explicit ks_basic_xmutable_string_base(const ks_basic_string_view<ELEM>& str_view);
	explicit ks_basic_xmutable_string_base(size_t count, ELEM ch);
	explicit ks_basic_xmutable_string_base(std::basic_string<ELEM, std::char_traits<ELEM>, ks_basic_string_allocator<ELEM>>&& str_rvref);

	//detach-void
	ks_basic_xmutable_string_base do_detach() {
		ks_basic_xmutable_string_base ret(std::move(*this));
		this->do_detach_void();
		return ret;
	}

	void do_detach_void() {
		this->~ks_basic_xmutable_string_base();
		::new (this) ks_basic_xmutable_string_base{};
	}

public:
	iterator begin() const { return iterator{ this->data() }; }
	iterator end() const { return iterator{ this->data_end() }; }
	const_iterator cbegin() const { return const_iterator{ this->data() }; }
	const_iterator cend() const { return const_iterator{ this->data_end() }; }
	reverse_iterator rbegin() const { return reverse_iterator{ this->end() }; }
	reverse_iterator rend() const { return reverse_iterator{ this->begin() }; }
	const_reverse_iterator crbegin() const { return reverse_iterator{ this->cend() }; }
	const_reverse_iterator crend() const { return reverse_iterator{ this->cbegin() }; }

protected:
	bool do_check_end_ch0() const { return this->data()[this->length()] == 0; }
	void do_ensure_end_ch0(bool ensure_end_ch0) {
		if (ensure_end_ch0 && !this->do_check_end_ch0()) {
			this->do_ensure_exclusive();
			this->unsafe_data()[this->length()] = 0;
		}
	}

	void do_ensure_exclusive();

	bool do_determine_need_grow(size_t grow) { return ptrdiff_t(grow) > 0 && this->length() + grow > this->capacity(); }
	void do_auto_grow(size_t grow);

	void do_reserve(size_t capa);

	void do_resize(size_t count, ELEM ch, bool ch_valid, bool ensure_end_ch0) {
		size_t old_length = this->length();
		if (count < old_length)
			return this->do_erase(count, old_length - count, ensure_end_ch0);
		else if (count > old_length)
			return this->do_append(count - old_length, ch, ch_valid, ensure_end_ch0);
	}

	void do_trim(bool ensure_end_ch0) {
		const auto this_view = this->view();
		auto trimmed_view = this_view.trimmed();
		if (trimmed_view.length() != this_view.length()) {
			*this = this->unsafe_substr(trimmed_view.data() - this_view.data(), trimmed_view.length());
			this->do_ensure_end_ch0(ensure_end_ch0);
		}
	}
	void do_trim_left(bool ensure_end_ch0) {
		const auto this_view = this->view();
		auto trimmed_view = this_view.trim_left();
		if (trimmed_view.length() != this_view.length()) {
			*this = this->unsafe_substr(trimmed_view.data() - this_view.data(), trimmed_view.length());
			this->do_ensure_end_ch0(ensure_end_ch0);
		}
	}
	void do_trim_right(bool ensure_end_ch0) {
		const auto this_view = this->view();
		auto trimmed_view = this_view.trim_right();
		if (trimmed_view.length() != this_view.length()) {
			*this = this->unsafe_substr(trimmed_view.data() - this_view.data(), trimmed_view.length());
			this->do_ensure_end_ch0(ensure_end_ch0);
		}
	}

	void do_shrink() {
		if (this->is_ref_mode()) {
			auto* ref_ptr = _my_ref_ptr();
			if (ref_ptr->offset32 != 0 ||
				ref_ptr->offset32 + ref_ptr->length32 != ks_basic_string_allocator<ELEM>::_get_space32_value(ref_ptr->alloc_addr()) - 1) {
				*this = ks_basic_xmutable_string_base(this->data(), this->length());
			}
		}
	}

protected:
	void do_assign(const ks_basic_string_view<ELEM>& str_view, bool ensure_end_ch0);
	void do_assign(size_t count, ELEM ch, bool ch_valid, bool ensure_end_ch0);

	void do_append(const ks_basic_string_view<ELEM>& str_view, bool ensure_end_ch0) {
		this->do_insert(this->length(), str_view, ensure_end_ch0);
	}
	void do_append(size_t count, ELEM ch, bool ch_valid, bool ensure_end_ch0) {
		this->do_insert(this->length(), count, ch, ch_valid, ensure_end_ch0);
	}

	void do_insert(size_t pos, const ks_basic_string_view<ELEM>& str_view, bool ensure_end_ch0);
	void do_insert(size_t pos, size_t count, ELEM ch, bool ch_valid, bool ensure_end_ch0);

	void do_replace(size_t pos, size_t number, const ks_basic_string_view<ELEM>& str_view, bool ensure_end_ch0);
	void do_replace(size_t pos, size_t number, size_t count, ELEM ch, bool ch_valid, bool ensure_end_ch0);

	size_t do_substitute_n(const ks_basic_string_view<ELEM>& sub, const ks_basic_string_view<ELEM>& replacement, size_t n, bool ensure_end_ch0);

	void do_erase(size_t pos, size_t number, bool ensure_end_ch0);
	void do_clear(bool ensure_end_ch0);

	template <class RIGHT, class _ = std::enable_if_t<std::is_convertible_v<RIGHT, ks_basic_string_view<ELEM>>>>
	void do_self_add(RIGHT&& right, bool could_ref_right_data_directly, bool ensure_end_ch0);

public:
	int compare(const ELEM* p) const { return this->view().compare(p); }
	int compare(const ELEM* p, size_t count) const { return this->view().compare(p, count); }
	int compare(const ks_basic_string_view<ELEM>& str_view) const { return this->view().compare(str_view); }

	bool contains(const ELEM* p) const { return this->view().contains(p); }
	bool contains(const ELEM* p, size_t count) const { return this->view().contains(p, count); }
	bool contains(const ks_basic_string_view<ELEM>& str_view) const { return this->view().contains(str_view); }
	bool contains(ELEM ch) const { return this->view().contains(ch); }

	bool starts_with(const ELEM* p) const { return this->view().starts_with(p); }
	bool starts_with(const ELEM* p, size_t count) const { return this->view().starts_with(p, count); }
	bool starts_with(const ks_basic_string_view<ELEM>& str_view) const { return this->view().starts_with(str_view); }
	bool starts_with(ELEM ch) const { return this->view().starts_with(ch); }

	bool ends_with(const ELEM* p) const { return this->view().ends_with(p); }
	bool ends_with(const ELEM* p, size_t count) const { return this->view().ends_with(p, count); }
	bool ends_with(const ks_basic_string_view<ELEM>& str_view) const { return this->view().ends_with(str_view); }
	bool ends_with(ELEM ch) const { return this->view().ends_with(ch); }

	size_t find(const ELEM* p, size_t pos = 0) const { return this->view().find(p, pos); }
	size_t find(const ELEM* p, size_t pos, size_t count) const { return this->view().find(p, pos, count); }
	size_t find(const ks_basic_string_view<ELEM>& str_view, size_t pos = 0) const { return this->view().find(str_view, pos); }
	size_t find(ELEM ch, size_t pos = 0) const { return this->view().find(ch, pos); }

	size_t rfind(const ELEM* p, size_t pos = -1) const { return this->view().rfind(p, pos); }
	size_t rfind(const ELEM* p, size_t pos, size_t count) const { return this->view().rfind(p, pos, count); }
	size_t rfind(const ks_basic_string_view<ELEM>& str_view, size_t pos = -1) const { return this->view().rfind(str_view, pos); }
	size_t rfind(ELEM ch, size_t pos = -1) const { return this->view().rfind(ch, pos); }

	size_t find_first_of(const ELEM* p, size_t pos = 0) const { return this->view().find_first_of(p, pos); }
	size_t find_first_of(const ELEM* p, size_t pos, size_t count) const { return this->view().find_first_of(p, pos, count); }
	size_t find_first_of(const ks_basic_string_view<ELEM>& str_view, size_t pos = 0) const { return this->view().find_first_of(str_view, pos); }
	size_t find_first_of(ELEM ch, size_t pos = 0) const { return this->view().find_first_of(ch, pos); }

	size_t find_last_of(const ELEM* p, size_t pos = -1) const { return this->view().find_last_of(p, pos); }
	size_t find_last_of(const ELEM* p, size_t pos, size_t count) const { return this->view().find_last_of(p, pos, count); }
	size_t find_last_of(const ks_basic_string_view<ELEM>& str_view, size_t pos = -1) const { return this->view().find_last_of(str_view, pos); }
	size_t find_last_of(ELEM ch, size_t pos = -1) const { return this->view().find_last_of(ch, pos); }

	size_t find_first_not_of(const ELEM* p, size_t pos = 0) const { return this->view().find_first_of(p, pos); }
	size_t find_first_not_of(const ELEM* p, size_t pos, size_t count) const { return this->view().find_first_of(p, pos, count); }
	size_t find_first_not_of(const ks_basic_string_view<ELEM>& str_view, size_t pos = 0) const { return this->view().find_first_of(str_view, pos); }
	size_t find_first_not_of(ELEM ch, size_t pos = 0) const { return this->view().find_first_of(ch, pos); }

	size_t find_last_not_of(const ELEM* p, size_t pos = -1) const { return this->view().find_last_of(p, pos); }
	size_t find_last_not_of(const ELEM* p, size_t pos, size_t count) const { return this->view().find_last_of(p, pos, count); }
	size_t find_last_not_of(const ks_basic_string_view<ELEM>& str_view, size_t pos = -1) const { return this->view().find_last_of(str_view, pos); }
	size_t find_last_not_of(ELEM ch, size_t pos = -1) const { return this->view().find_last_of(ch, pos); }

protected:
	ks_basic_xmutable_string_base do_slice(size_t from, size_t to) const {
		const size_t this_length = this->length();
		if (from > this_length)
			from = this_length;
		if (to > this_length)
			to = this_length;
		else if (to < from)
			to = from;
		return this->unsafe_substr(from, to - from);
	}

	ks_basic_xmutable_string_base do_substr(size_t pos, size_t count) const {
		const size_t this_length = this->length();
		if (pos > this_length)
			throw std::out_of_range("ks_basic_xmutable_string_base::substr(pos, count) out-of-range exception");
		if (count > this_length - pos)
			count = this_length - pos;
		return this->unsafe_substr(pos, count);
	}

protected:
	ks_basic_xmutable_string_base unsafe_substr(size_t pos, size_t count) const {
		ASSERT(this->view().unsafe_subview(pos, count + 1).is_subview_of(this->unsafe_whole_view()));
		if (count <= _SSO_BUFFER_SPACE - 1) {
			return ks_basic_xmutable_string_base(this->view().data() + (ptrdiff_t)pos, count);
		}
		else {
			ks_basic_xmutable_string_base slice = *this;
			ASSERT(slice.is_ref_mode());
			auto* slice_ref_ptr = slice._my_ref_ptr();
			slice_ref_ptr->p += (ptrdiff_t)pos;
			slice_ref_ptr->offset32 += (int32_t)(ptrdiff_t)pos;
			slice_ref_ptr->length32 = (uint32_t)count;
			return slice;
		}
	}

	ks_basic_string_view<ELEM> unsafe_whole_view() const {
		if (this->is_sso_mode()) {
			auto* sso_ptr = _my_sso_ptr();
			return ks_basic_string_view<ELEM>(sso_ptr->buffer, _SSO_BUFFER_SPACE);
		}
		else {
			auto* ref_ptr = _my_ref_ptr();
			ELEM* alloc_addr = ref_ptr->alloc_addr();
			return ks_basic_string_view<ELEM>(alloc_addr, ks_basic_string_allocator<ELEM>::_get_space32_value(alloc_addr));
		}
	}

protected:
	template <class STR_TYPE, class _ = std::enable_if_t<std::is_base_of_v<ks_basic_xmutable_string_base<ELEM>, STR_TYPE>>>
	std::vector<STR_TYPE> do_split(const ks_basic_string_view<ELEM>& sep, size_t n) const;

public:
	const ELEM& front() const {
		ASSERT(!this->empty());
		return *this->data();
	}

	const ELEM& back() const {
		ASSERT(!this->empty());
		return *(this->data_end() - 1);
	}

	const ELEM& at(size_t pos) const {
		if (pos >= this->length())
			throw std::out_of_range("ks_basic_xmutable_string_base::at(pos) out-of-range exception");
		else
			return this->data()[pos];
	}

	const ELEM& operator[](size_t pos) const {
		ASSERT(pos < this->length());
		return this->data()[pos];
	}

public:
	const ELEM* data() const {
		return this->is_sso_mode()
			? _my_sso_ptr()->buffer
			: _my_ref_ptr()->p;
	}

	const ELEM* data_end() const {
		return this->is_sso_mode()
			? _my_sso_ptr()->buffer + _my_sso_ptr()->length8
			: _my_ref_ptr()->p + _my_ref_ptr()->length32;
	}

	size_t length() const {
		return this->is_sso_mode()
			? _my_sso_ptr()->length8
			: _my_ref_ptr()->length32;
	}

	size_t size() const { return this->length(); }
	bool empty() const { return this->length() == 0; }

	size_t capacity() const {
		return this->is_sso_mode()
			? _SSO_BUFFER_SPACE - 1
			: (ks_basic_string_allocator<ELEM>::_get_space32_value(_my_ref_ptr()->alloc_addr()) - 1) - _my_ref_ptr()->offset32;
	}

	bool is_exclusive() const {
		return !(this->is_ref_mode() && ks_basic_string_allocator<ELEM>::_peek_refcount32_value(_my_ref_ptr()->alloc_addr(), false) > 1); //note: no need to acquire
	}

	ks_basic_string_view<ELEM> view() const {
		return ks_basic_string_view<ELEM>(this->data(), this->length());
	}

protected:
	bool is_sso_mode() const { return _my_mode() == _SSO_MODE; }
	bool is_ref_mode() const { return _my_mode() == _REF_MODE; }
	bool is_detached_empty() const { return this->is_sso_mode() && this->empty(); }

	ELEM* unsafe_data() const { return (ELEM*)this->data(); }
	ELEM* unsafe_data_end() const { return (ELEM*)this->data_end(); }

private:
	static constexpr uint8_t _SSO_MODE = 0;
	static constexpr uint8_t _REF_MODE = 1;

	static constexpr size_t _MODE_BITS = 1;
	static constexpr size_t _FIX_DATA_SIZE = std::max(size_t(sizeof(ELEM) <= 2 ? 16 : 24), (sizeof(ELEM) * 2) / 8 * 8); //use 32 is good also, and std::basic_string uses just 32, but we use smaller size for mem-compact
	static constexpr size_t _SSO_BUFFER_SPACE = ((_FIX_DATA_SIZE - 2) / sizeof(ELEM)); //why sub 2? see also _SSO_STRUCT
	static constexpr size_t _STR_LENGTH_LIMIT = 0x7FFFFFFF; //due to _REF_STRUCT::offset32 is 31 bits actually, so the max len is defined as here
	static_assert(_SSO_BUFFER_SPACE != 0, "sso-buffer-space must not be 0");

	struct _SSO_STRUCT {
		uint8_t mode : _MODE_BITS;
		uint8_t length8; //due to gap exists, length8 need not share same byte with mode, for optimization
		ELEM buffer[_SSO_BUFFER_SPACE];
	};
	struct _REF_STRUCT {
		uint32_t mode : _MODE_BITS;
		uint32_t offset32 : (32 - _MODE_BITS);
		uint32_t length32;
		ELEM* p;
		ELEM* alloc_addr() const { return this->p - (size_t)(this->offset32); }
	};

	union _DATA_UNION {
		uint8_t mode : _MODE_BITS;
		_SSO_STRUCT sso_struct;
		_REF_STRUCT ref_struct;
	};

	static_assert(sizeof(_SSO_STRUCT) <= _FIX_DATA_SIZE, "the size of SSO_STRUCT is not perfect");
	static_assert(sizeof(_REF_STRUCT) <= _FIX_DATA_SIZE, "the size of REF_STRUCT it not perfect");
	static_assert(sizeof(_DATA_UNION) <= _FIX_DATA_SIZE, "the size of DATA_UNION is not perfect");

	_DATA_UNION m_data_union;

private:
	uint8_t _my_mode() const { return m_data_union.mode; }
	_SSO_STRUCT* _my_sso_ptr() { return &m_data_union.sso_struct; }
	_REF_STRUCT* _my_ref_ptr() { return &m_data_union.ref_struct; }
	const _SSO_STRUCT* _my_sso_ptr() const { return &m_data_union.sso_struct; }
	const _REF_STRUCT* _my_ref_ptr() const { return &m_data_union.ref_struct; }

public:
	bool operator==(const ks_basic_string_view<ELEM>& right) const { return this->view() == right; }
	bool operator!=(const ks_basic_string_view<ELEM>& right) const { return this->view() != right; }
	bool operator<(const ks_basic_string_view<ELEM>& right) const { return this->view() < right; }
	bool operator<=(const ks_basic_string_view<ELEM>& right) const { return this->view() <= right; }
	bool operator>(const ks_basic_string_view<ELEM>& right) const { return this->view() > right; }
	bool operator>=(const ks_basic_string_view<ELEM>& right) const { return this->view() >= right; }

protected: 
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ELEM* p) { return ks_basic_string_view<ELEM>::__to_basic_string_view(p); }
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ELEM* p, size_t count) { return ks_basic_string_view<ELEM>::__to_basic_string_view(p, count); }

	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ks_basic_string_view<ELEM>& str_view) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str_view); }
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ks_basic_string_view<ELEM>& str_view, size_t offset, size_t count) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str_view, offset, count); }

	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ks_basic_xmutable_string_base<ELEM>& str) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str); }
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const ks_basic_xmutable_string_base<ELEM>& str, size_t offset, size_t count) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str, offset, count); }

	template <class CharTraits, class AllocType>
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const std::basic_string<ELEM, CharTraits, AllocType>& str) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str); }
	template <class CharTraits, class AllocType>
	static constexpr inline ks_basic_string_view<ELEM> __to_basic_string_view(const std::basic_string<ELEM, CharTraits, AllocType>& str, size_t offset, size_t count) { return ks_basic_string_view<ELEM>::__to_basic_string_view(str, offset, count); }

	friend class ks_basic_mutable_string<ELEM>;
	friend class ks_basic_immutable_string<ELEM>;
};


#include "ks_basic_xmutable_string_base.inl"
