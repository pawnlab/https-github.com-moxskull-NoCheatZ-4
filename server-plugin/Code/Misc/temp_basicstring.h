#ifndef TEMP_BASICSTRING
#define TEMP_BASICSTRING

#include <new>
#include <string.h>
#include <limits>

#include "Containers/utlvector.h"

template <typename pod = char>
class String
{
	friend String<char>;
	friend String<wchar_t>;

public:
	constexpr static size_t const npos = std::numeric_limits<size_t>::max();

private:

	/*  -need- always contains the zero char. */
	void Grow(size_t need, bool copy = true)
	{
		if (need <= m_capacity)
			return;

		size_t new_capacity = 32;
		while (new_capacity < need) new_capacity <<= 1;

		pod *n = new pod[new_capacity];
		
		if (m_alloc)
		{
			if(copy) memcpy(n, m_alloc, m_size * sizeof(pod));
			delete[] m_alloc;
		}
		else
		{
			n[0] = 0;
			m_size = 0;
		}

		m_alloc = n;
		m_capacity = new_capacity;
	}

	pod * m_alloc;
	size_t m_size;
	size_t m_capacity;

private:
	static size_t autolen(pod const * string)
	{
		return strlen(string);
	}

	static size_t autonlen(pod const * string, size_t max)
	{
		return strnlen(string, max);
	}

	static pod const * autoempty()
	{
		return "";
	}

public:

	String()
	{
		memset(this, 0, sizeof(String));
	}

	~String()
	{
		if (m_alloc)
			delete[] m_alloc;
#ifdef DEBUG
		memset(this, 0xCCCCCCCC, sizeof(String));
#endif
	}

	const pod *c_str() const
	{
		return m_alloc ? m_alloc : autoempty();
	}

	void assign(const String<pod> &src)
	{
		assign(src.c_str());
	}

	void clear()
	{
		if (m_alloc)
			m_alloc[0] = 0;
		m_size = 0;
	}

	void assign(pod const *d)
	{
		if (!d)
		{
			clear();
		}
		else
		{
			size_t const len = autolen(d) + 1;
			Grow(len, false);
			memcpy(m_alloc, d, len * sizeof(pod));
			m_size = autolen(m_alloc);
		}
	}

	void assign(pod const *d, size_t count)
	{
		if (!d)
		{
			clear();
		}
		else
		{
			size_t const len = autonlen(d, count) + 1;
			Grow(len, false);
			memcpy(m_alloc, d, len * sizeof(pod));
			m_alloc[len] = 0;
			m_size = autolen(m_alloc);
		}
	}

	String(pod const *src) : String()
	{
		assign(src);
	}

	String(pod const *src, size_t start, size_t count = std::numeric_limits<size_t>::max()) : String()
	{
		assign(src+start, count);
	}

	String(String<pod> const &src) : String()
	{
		assign(src.c_str());
	}

	String(String<pod> const &src, size_t start, size_t count = std::numeric_limits<size_t>::max()) : String()
	{
		assign(src.c_str()+start, count);
	}

	String & operator = (String<pod> const &src)
	{
		assign(src);
		return *this;
	}

	String & operator = (pod const *src)
	{
		assign(src);
		return *this;
	}

	bool operator ==(const String<pod> &other) const
	{
		if (m_size != other.m_size) return false;

		pod const * me = m_alloc;
		pod const * other_c = other.m_alloc;
		do
		{
			if (*me != *other_c) return false;
			++other_c;
		} while (*me++ != 0);

		return true;
	}

	inline bool operator !=(const String<pod> &other) const
	{
		return !this->operator==(other);
	}

	void reserve(size_t len)
	{
		Grow(len+1);
	}

	bool operator ==(pod const * other) const
	{
		if (m_size != autolen(other)) return false;

		pod const * me = m_alloc;
		do
		{
			if (*me != *other) return false;
			++other;
		} while (*me++ != 0);

		return true;
	}

	inline bool operator !=(pod const * other) const
	{
		return !this->operator==(other);
	}

	String<pod>& append(pod const * t)
	{
		size_t const len = autolen(t) + 1;
		Grow(m_size + len);
		memcpy(m_alloc + m_size, t, sizeof(pod) * len);
		m_size = strlen(m_alloc);
		return *this;
	}

	String<pod>& append(pod const c)
	{
		size_t pos = m_size;
		Grow(pos + 2); // should be + 1 but when m_alloc is not allocated we also need to add '\0'
		m_alloc[pos] = c;
		++pos;
		m_alloc[pos] = 0;
		++m_size;
		return *this;
	}

	String<pod>& append(String<pod> const & d)
	{
		append(d.c_str());
		return *this;
	}

	bool isempty() const
	{
		return m_size == 0;
	}

	size_t size() const
	{
		return m_size;
	}

	size_t length() const
	{
		return m_size;
	}

	String<pod>& replace(pod const replace_this, pod const replace_by)
	{
		if (m_size > 0)
		{
			pod * me = m_alloc;
			do
			{
				if (*me == replace_this) *me = replace_by;
			} while (*++me != 0);
		}

		return *this;
	}

	String<pod>& replace(pod const * replace_list, pod const replace_by)
	{
		if (m_size > 0)
		{
			while (*replace_list != 0)
			{
				replace(*replace_list, replace_by);
				++replace_list;
			}
		}

		return *this;
	}

	String<pod>& remove(size_t pos)
	{
		if (pos < m_size)
		{
			do
			{
				m_alloc[pos] = m_alloc[pos + 1];
			} while (m_alloc[++pos] != 0);
			--m_size;
		}
		return *this;
	}

	String<pod>& remove(size_t const start, size_t end)
	{
		if (start > end) return remove(end, start);
		if (end < m_size)
		{
			do
			{
				remove(end);
			} while (end-- - start != 0);
		}
		return *this;
	}

	String<pod>& replace(String<pod> const & replace_this, String<pod> const & replace_by)
	{
		int const diff = replace_by.m_size - replace_this.m_size;
		size_t pos = find(replace_this);
		while (pos != npos)
		{
			if (diff <= 0)
			{
				memcpy(m_alloc + pos, replace_by.m_alloc, sizeof(pod) * replace_by.m_size - 1);
				if(diff < 0) remove(pos + replace_by.m_size, pos + replace_this.m_size - 1);
			}
			else if (diff > 0)
			{
				memcpy(m_alloc + pos, replace_by.m_alloc, sizeof(pod) * replace_this.m_size - 1);
				Grow(m_size + diff + 1);
				size_t move_from_here = m_size + diff;
				size_t const move_until_here = pos + replace_this.m_size - 1;
				do
				{
					m_alloc[move_from_here] = m_alloc[move_from_here - diff];
				} while (--move_from_here != move_until_here);
				memcpy(m_alloc + pos + replace_this.m_size, replace_by.m_alloc + replace_this.m_size, sizeof(pod) * diff);
				m_size += diff;
			}
			pos = find(replace_this, pos);
		}
		return *this;
	}

	size_t find(pod const c, size_t start = 0) const
	{
		size_t a = m_size;
		if (a == 0) return npos;
		if (start >= a) return npos;
		a = start;
		pod const * me = m_alloc + start;
		do
		{
			if (*me == c) return a;
			++a;
		} while (*(++me) != 0);

		return npos;
	}

	size_t find(pod const * const c, size_t start = 0) const
	{
		size_t const csize = autolen(c);
		if (start + csize > m_size) return npos;
		char * me = m_alloc + start;

		while (memcmp(me, c, csize * sizeof(char)) != 0)
		{
			++me;
			if (++start + csize > m_size) return npos;
		}

		return start;
	}

	size_t find(String<pod> const & c, size_t start = 0) const
	{
		return find(c.c_str());
	}

	size_t find_last_of(pod const * c, size_t start = npos) const
	{
		String const temp(c);

		size_t a = m_size;
		if (a == 0) return npos;
		if (start == 0) return npos;

		pod const * me = m_alloc + a - 1;
		do
		{
			if (temp.find(*me) != npos) return a;
			--a;
		} while (me-- != m_alloc);

		return npos;
	}

	size_t find_last_of(pod const c, size_t start = npos) const
	{
		size_t a = m_size;
		if (a == 0) return npos;
		if (start == 0) return npos;

		pod const * me = m_alloc + a - 1;
		do
		{
			if (*me == c) return a;
			--a;
		} while (me-- != m_alloc);

		return npos;
	}

	void lower()
	{
		if (!m_alloc)
			return;
		pod * me = m_alloc;
		while (*me != 0)
		{
			if (isupper(*me)) 
				*me = (pod)tolower(*me);
			++me;
		} 
	}

	pod& operator[] (size_t index) const
	{
		return m_alloc[index];
	}

	static void ConvertToWideChar(String<char> const & in, String<wchar_t> & out)
	{
		out.Grow(in.m_size, false);
		size_t const cpsize = mbstowcs(out.m_alloc, in.m_alloc, out.m_capacity);
		Assert(cpsize < std::numeric_limits<size_t>::max());
		out.m_size = cpsize;
	}
};

size_t String<wchar_t>::autolen(wchar_t const * string)
{
	return wcslen(string);
}

size_t String<wchar_t>::autonlen(wchar_t const * string, size_t max)
{
	return wcsnlen(string, max);
}

wchar_t const * String<wchar_t>::autoempty()
{
	return L"";
}

typedef String<char> basic_string;
typedef String<wchar_t> basic_wstring;

template <typename pod>
void SplitString(String<pod> const & string, pod const delim, CUtlVector < String<pod> > & out)
{
	out.RemoveAll();
	out.EnsureCapacity(8);

	size_t start = 0;
	size_t end;
	for(;;)
	{
		end = string.find(delim, start);
		if (end == String<pod>::npos)
		{
			out.AddToTail(String<pod>(string, start));
			break;
		}
		else
		{
			out.AddToTail(String<pod>(string, start, end - 1 - start));
		}
		start = end+1;
	}

}

#endif

