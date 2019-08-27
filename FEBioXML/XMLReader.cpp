// XMLReader.cpp: implementation of the XMLReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLReader.h"
#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------
// XMLTag
//-----------------------------------------------------------------------------
XMLAtt::XMLAtt()
{
	m_szatt[0] = 0;
	m_szatv[0] = 0;
}

//-----------------------------------------------------------------------------
bool XMLAtt::operator == (const char* sz)
{
	return (strcmp(m_szatv, sz) == 0);
}

//////////////////////////////////////////////////////////////////////
// XMLTag
//////////////////////////////////////////////////////////////////////

XMLTag::XMLTag()
{
	m_preader = 0;
	m_bend = false;

	m_sztag[0] = 0;
	m_nlevel = 0;

	m_natt = 0;

	for (int i=0; i<XMLReader::MAX_LEVEL; ++i) m_szroot[i][0] = 0;
}

//-----------------------------------------------------------------------------
//! Clear tag data
void XMLTag::clear()
{
	m_sztag[0] = 0;
	m_szval.clear();
	m_natt = 0;
	m_bend = false;
	m_bleaf = true;
	m_bempty = false;
}

//-----------------------------------------------------------------------------
//! This function reads in a comma delimited list of doubles. The function reads
//! in a maximum of n values. The actual number of values that are read is returned.
//!
int XMLTag::value(double* pf, int n)
{
	const char* sz = m_szval.c_str();
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		const char* sze = strchr(sz, ',');

		pf[i] = atof(sz);
		nr++;

		if (sze) sz = sze+1;
		else break;
	}
	return nr;
}

//-----------------------------------------------------------------------------
//! This function reads in a comma delimited list of floats. The function reads
//! in a maximum of n values. The actual number of values that are read is returned.
//!
int XMLTag::value(float* pf, int n)
{
	const char* sz = m_szval.c_str();
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		const char* sze = strchr(sz, ',');

		pf[i] = (float) atof(sz);
		nr++;

		if (sze) sz = sze+1;
		else break;
	}
	return nr;
}

//-----------------------------------------------------------------------------
//! This function reads in a comma delimited list of ints. The function reads
//! in a maximum of n values. The actual number of values that are read is returned.
//!
int XMLTag::value(int* pi, int n)
{
	const char* sz = m_szval.c_str();
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		const char* sze = strchr(sz, ',');

		pi[i] = atoi(sz);
		nr++;

		if (sze) sz = sze+1;
		else break;
	}
	return nr;
}

//////////////////////////////////////////////////////////////////////

void XMLTag::value(bool& val)
{ 
	int n=0; 
	int x=sscanf(m_szval.c_str(), "%d", &n); 
	val = (n != 0); 
}

void XMLTag::value(char* szstr)
{
	strcpy(szstr, m_szval.c_str()); 
}

void XMLTag::value(vec3d& v)
{
	int n = sscanf(m_szval.c_str(), "%lg,%lg,%lg", &v.x, &v.y, &v.z);
	if (n != 3) throw XMLReader::XMLSyntaxError();
}

void XMLTag::value(mat3d& m)
{
	double xx, xy, xz, yx, yy, yz, zx, zy, zz;
	int n = sscanf(m_szval.c_str(), "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);
	if (n != 9) throw XMLReader::XMLSyntaxError();
	m = mat3d(xx, xy, xz, yx, yy, yz, zx, zy, zz);
}

void XMLTag::value(mat3ds& m)
{
	double x, y, z, xy, yz, xz;
	int n = sscanf(m_szval.c_str(), "%lg,%lg,%lg,%lg,%lg,%lg", &x, &y, &z, &xy, &yz, &xz);
	if (n != 6) throw XMLReader::XMLSyntaxError();
	m = mat3ds(x, y, z, xy, yz, xz);
}

//-----------------------------------------------------------------------------
void XMLTag::value(vector<int>& l)
{
	int i, n = 0, n0, n1, nn;
	char* szval = strdup(m_szval.c_str());
	char* ch;
	char* sz = szval;
	int nread;
	do
	{
		ch = strchr(sz, ',');
		if (ch) *ch = 0;
		nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
		switch (nread)
		{
		case 1:
			n1 = n0;
			nn = 1;
			break;
		case 2:
			nn = 1;
			break;
		case 3:
			break;
		default:
			n0 = 0;
			n1 = -1;
			nn = 1;
		}

		for (i=n0; i<=n1; i += nn) ++n;

		if (ch) *ch = ',';
		sz = ch+1;
	}
	while (ch != 0);

	if (n != 0)
	{
		l.resize(n);

		sz = szval;
		n = 0;
		do
		{
			ch = strchr(sz, ',');
			if (ch) *ch = 0;
			nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
			switch (nread)
			{
			case 1:
				n1 = n0;
				nn = 1;
				break;
			case 2:
				nn = 1;
			}

			for (i=n0; i<=n1; i += nn) l[n++] = i;
			assert(n <= (int) l.size());

			if (ch) *ch = ',';
			sz = ch+1;
		}
		while (ch != 0);
	}

	free(szval);
}

//-----------------------------------------------------------------------------
//! Return the number of children of a tag
int XMLTag::children()
{
	XMLTag tag(*this); ++tag;
	int ncount = 0;
	while (!tag.isend()) { ncount++; m_preader->SkipTag(tag); }
	return ncount;
}

//-----------------------------------------------------------------------------
//! return the string value of an attribute
const char* XMLTag::AttributeValue(const char* szat, bool bopt)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_szatt, szat) == 0) return m_att[i].m_szatv;

	// If the attribute was not optional, we throw a fit
	if (!bopt) throw XMLReader::MissingAttribute(*this, szat);

	// we didn't find it
	return 0;
}

//-----------------------------------------------------------------------------
//! return the attribute
XMLReader::XMLAtt* XMLTag::Attribute(const char* szat, bool bopt)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_szatt, szat) == 0) return m_att+i;

	// If the attribute was not optional, we throw a fit
	if (!bopt) throw XMLReader::MissingAttribute(*this, szat);

	// we didn't find it
	return 0;
}

//-----------------------------------------------------------------------------
//! return the attribute
XMLReader::XMLAtt& XMLTag::Attribute(const char* szat)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_szatt, szat) == 0) return m_att[i];

	// throw a fit
	throw XMLReader::MissingAttribute(*this, szat);
}

//-----------------------------------------------------------------------------
bool XMLTag::AttributeValue(const char* szat, double& d, bool bopt)
{
	const char* szv = AttributeValue(szat, bopt);
	if (szv == 0) return false;

	d = atof(szv);

	return true;
}

//////////////////////////////////////////////////////////////////////

bool XMLTag::AttributeValue(const char* szat, int& n, bool bopt)
{
	const char* szv = AttributeValue(szat, bopt);
	if (szv == 0) return false;

	n = atoi(szv);

	return true;
}

//////////////////////////////////////////////////////////////////////
// XMLReader - Exceptions
//////////////////////////////////////////////////////////////////////

XMLReader::InvalidAttributeValue::InvalidAttributeValue(XMLReader::XMLTag& t, const char* sza, const char* szv) : tag(t)
{ 
	strcpy(szatt, sza);
	if (szv) strcpy(szval, szv); else szval[0] = 0;
}

XMLReader::MissingAttribute::MissingAttribute(XMLTag& t, const char* sza) : tag(t)
{ 
	strcpy(szatt, sza); 
}

//////////////////////////////////////////////////////////////////////
// XMLReader
//////////////////////////////////////////////////////////////////////

XMLReader::XMLReader()
{
	m_fp = 0;
	m_nline = 0;
}

XMLReader::~XMLReader()
{
	Close();
}

//////////////////////////////////////////////////////////////////////

void XMLReader::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = 0;
		m_nline = 0;
	}
}

//////////////////////////////////////////////////////////////////////

bool XMLReader::Open(const char* szfile)
{
	// make sure this reader has not been attached to a file yet
	if (m_fp != 0) return false;

	// open the file
	m_fp = fopen(szfile, "rb");
	if (m_fp == 0) return false;

	// read the first line
	char szline[256] = {0};
	fgets(szline, 255, m_fp);

	// make sure it is correct
	if (strncmp(szline,"<?xml", 5) != 0)
	{
		// This file is not an XML file
		return false;
	}

	// This file is ready to be processed
	return true;
}

//////////////////////////////////////////////////////////////////////

bool XMLReader::FindTag(const char* sztag, XMLTag& tag)
{
	// go to the beginning of the file
	fseek(m_fp, 0, SEEK_SET);

	// set the first tag
	tag.m_preader = this;
	tag.m_ncurrent_line = 1;
	fgetpos(m_fp, &tag.m_fpos);

	// find the correct tag
	try
	{
		bool bfound = false;
		do
		{
			NextTag(tag);
			if (strcmp(sztag, tag.m_sztag) == 0) bfound = true;
		}
		while (!bfound);
	}
	catch (...)
	{
		// an error has occured (or the end-of-file was reached)
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////

void XMLReader::NextTag(XMLTag& tag)
{
	assert( tag.m_preader == this);

	// set current line number
	m_nline = tag.m_ncurrent_line;

	// set the current file position
	fsetpos(m_fp, &tag.m_fpos);

	// clear tag's content
	tag.clear();

	// read the start tag
	ReadTag(tag);

	try
	{
		// read value and end tag if tag is not empty
		if (!tag.isempty())
		{
			// read the value
			ReadValue(tag);

			// read the end tag
			ReadEndTag(tag);
		}
	}
	catch (EndOfFile)
	{
		if (!tag.isend()) throw UnexpectedEOF();
	}

	// store current line number
	tag.m_ncurrent_line = m_nline;

	// store start file pos for next element
	fgetpos(m_fp, &tag.m_fpos);
}

inline bool isvalid(char c)
{
	return (isalnum(c) || (c=='_') || (c=='.'));
}

void XMLReader::ReadTag(XMLTag& tag)
{
	// find the start token
	char ch, *sz;
	while (true)
	{
		while ((ch=GetChar())!='<') if (!isspace(ch)) throw XMLSyntaxError();

		ch = GetChar();
		if (ch == '!')
		{
			// parse the comment
			ch = GetChar(); if (ch != '-') throw XMLSyntaxError();
			ch = GetChar(); if (ch != '-') throw XMLSyntaxError();

			// find the end of the comment
			do
			{
				ch = GetChar();
				if (ch == '-')
				{
					ch = GetChar();
					if (ch == '-')
					{
						ch = GetChar();
						if (ch == '>') break;
					}
				}
			}
			while (1);
		}
		else if (ch == '?')
		{
			// parse the xml header tag
			while ((ch = GetChar()) != '?');
			ch = GetChar();
			if (ch != '>') throw XMLSyntaxError();
		}
		else break;
	}

	// record the startline
	tag.m_nstart_line = m_nline;

	if (ch == '/')
	{
		tag.m_bend = true;
		ch = GetChar();
	}

	// skip whitespace
	while (isspace(ch)) ch = GetChar();

	// read the tag name
	if (!isvalid(ch)) throw XMLSyntaxError();
	sz = tag.m_sztag;
	*sz++ = ch;
	while (isvalid(ch=GetChar())) *sz++ = ch;
	*sz = 0;

	// read attributes
	tag.m_natt = 0;
	int n = 0;
	sz = 0;
	while (true)
	{
		// skip whitespace
		while (isspace(ch)) ch = GetChar();
		if (ch == '/')
		{
			tag.m_bempty = true;
			ch = GetChar();
			if (ch != '>') throw XMLSyntaxError();
			break;
		}
		else if (ch == '>') break;

		// read the attribute's name
		sz = tag.m_att[n].m_szatt;
		if (!isvalid(ch)) throw XMLSyntaxError();
		*sz++ = ch;
		while (isvalid(ch=GetChar())) *sz++ = ch;
		*sz=0; sz=0;

		// skip whitespace
		while (isspace(ch)) ch=GetChar();
		if (ch != '=') throw XMLSyntaxError();

		// skip whitespace
		while (isspace(ch=GetChar()));
		if (ch != '"') throw XMLSyntaxError();

		sz = tag.m_att[n].m_szatv;
		while ((ch=GetChar())!='"') *sz++ = ch;
		*sz=0; sz=0;
		ch=GetChar();

		++n;
		++tag.m_natt;
	}

	if (!tag.isend() && !tag.isempty())
	{
		// keep a copy of the name
		strcpy(tag.m_szroot[tag.m_nlevel], tag.m_sztag);
		tag.m_nlevel++;
	}
}

void XMLReader::ReadValue(XMLTag& tag)
{
	char ch;
	if (!tag.isend())
	{
		tag.m_szval.clear();
		while ((ch=GetChar())!='<') 
		{ 
			tag.m_szval.push_back(ch);
		}
		tag.m_szval.push_back(0);
	}
	else while ((ch=GetChar())!='<');
}

void XMLReader::ReadEndTag(XMLTag& tag)
{
	char ch, *sz = tag.m_sztag;
	if (!tag.isend())
	{
		ch = GetChar();
		if (ch == '/')
		{
			// this is the end tag
			// make sure it matches the tag name
			--tag.m_nlevel;

			// skip whitespace
			while (isspace(ch=GetChar()));

			int n = 0;
			do
			{
				if (ch != *sz++) throw UnmatchedEndTag(tag);
				ch = GetChar();
				++n;
			}
			while (!isspace(ch) && (ch!='>'));
			if (n != (int) strlen(tag.m_sztag)) throw UnmatchedEndTag(tag);

			// skip whitespace
			while (isspace(ch)) ch=GetChar();
			if (ch != '>') throw XMLSyntaxError();

			// find the start of the next tag
			if (tag.m_nlevel)
			{
				while (isspace(ch=GetChar()));
				if (ch != '<') throw XMLSyntaxError();
				fseek(m_fp, -1, SEEK_CUR);
			}
		}
		else
		{
			// this element has child elements
			// and therefor is not a leaf

			tag.m_bleaf = false;
			fseek(m_fp, -2, SEEK_CUR);
		}
	}
	else
	{
		fseek(m_fp, -1, SEEK_CUR);

		--tag.m_nlevel;

		// make sure the name is the same as the root
		if (strcmp(tag.m_sztag, tag.m_szroot[tag.m_nlevel]) != 0) throw UnmatchedEndTag(tag);
	}
}

//-----------------------------------------------------------------------------
//! Read the next character in the file.
char XMLReader::GetChar()
{
	char ch;
	while ((ch=fgetc(m_fp))=='\n') ++m_nline;
	if (feof(m_fp)) 
	{
		int a = 0;
		throw EndOfFile();
	}
	return ch;
}

//-----------------------------------------------------------------------------
//! Skip a tag
void XMLReader::SkipTag(XMLTag& tag)
{
	// if this tag is a leaf we just return
	if (tag.isleaf()) { ++tag; return; }

	// if it is not a leaf we have to loop over all 
	// the children, skipping each child in turn
	NextTag(tag);
	do
	{
		SkipTag(tag);
	}
	while (!tag.isend());

	++tag;
}
