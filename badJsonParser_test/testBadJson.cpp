

// Copyright 2020 Alan Tracey Wootton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


// #define _CRTDBG_MAP_ALLOC
// #include <stdlib.h>
// #include <crtdbg.h>

#include <iostream>

#include <string>

#include "badjson.h"
#include "knotbase64.h"

#include <stdio.h>

using namespace mqtt5nano; // we'll use slices

using namespace std;
using namespace badjson;

char buffer[4 * 1024];
char buffer2[4 * 1024];
//sink dest = sink(buffer, sizeof(buffer));

SinkDrain dest(buffer2,sizeof(buffer));

void test1(string input, string want); // below

void test2(string input, string want); // below

void testLeak();

void suite1(); // below

int main()
{

	// test the getJSONinternal and the various Raw and GetQuoted
	const char *inP = "abc def";
	ResultsTriplette res = Chop(inP, strlen(inP));
	dest.buffer.reset();
	badjson::ToString(*res.segment,dest);//
	dest.writeByte(0);
	cout << "getJSONinternal returned " << dest.buffer.base << "\n";

	// TODO: test the parent, hex and base64 types.

	test2("\"a b c\"", "[a b c]");
	test2("a b c", "[a b c]");

	// TODO: test the array routines.
	// TODO: test the utf-8 characters. (which seem to work)

	test1("{a b}}", "[{\"a\":\"b\"},\"}\"]");

	suite1();

	testLeak();

	//_CrtDumpMemoryLeaks(); ?? 
}

void suite1()
{

	test1(" a ", "[\"a\"]"); //

	test1("abc", "[\"abc\"]");

	test1("123:456 abc:def,ghi:jkl", "[\"123\",\"456\",\"abc\",\"def\",\"ghi\",\"jkl\"]");

	test1("\"abc\"\"def\"\"ghi\"\"jkl\" ", "[\"abc\",\"def\",\"ghi\",\"jkl\"]"); //

	test1(" \"astr\" ", "[\"astr\"]"); //

	test1(" q\"str ", "[\"q\\\"str\"]"); // q"str not surrounded by "

	test1("   \"q\\\"str\"   ", "[\"q\\\"str\"]"); // q"str surrounded by "

	test1(" q1'str ", "[\"q1'str\"]"); // q'str

	test1("   \"q1'str\"   ", "[\"q1'str\"]"); // "q1'str"

	test1("  'str' ", "[\"str\"]"); // str with single q

	test1("  'st\\\'r' ", "[\"st'r\"]"); //

	test1("  's\\\'t\\\'r' ", "[\"s't'r\"]"); // s't'r in single q

	test1(" a\\b ", "[\"a\\\\b\"]"); //  a\b is \ in the middle of an unquoted str.
}

// parse the string and then outut in the json format
// and compair with the expected result.

void test1(string input, string want)
{
	const char *inP = input.c_str();

	ResultsTriplette res = Chop(inP, input.length());

	if (res.error || res.segment->input.empty())
	{
		if (res.error)
			cout << "FAIL res.error" << res.error << "\n";
		if (!res.segment->input.empty())
			cout << "FAIL res.input" << res.segment->input.getCstr(buffer2, sizeof(buffer2)) << "\n";
	}
	dest.reset();
	bool ok = ToString(*res.segment, dest);
	if (!ok)
	{
		cout << "FAIL ToString not ok \n ";
	}

	string got = dest.getWritten().getCstr(buffer2, sizeof(buffer2));

	cout << "got: " << got << "\n";
	if (got != want)
	{
		cout << "FAIL got: " << got << " but wanted" << want << "\n";
	}

	// now parse the result
	// and see if it's still good.

	res = Chop(got.c_str(), got.length());

	dest.reset();

	ok = ToString(*res.segment, dest);
	if (!ok)
	{
		cout << "FAIL ToString not ok \n ";
	}
	string got2 = dest.getWritten().getCstr(buffer2, sizeof(buffer2));
	if (got != got2)
	{
		cout << "FAIL pass2 got:  " << got << " got2: " << got2 << "\n";
	}

	delete res.segment;
}

void test2(string input, string want)
{
	// just check the first segment
	const char *inP = input.c_str();

	ResultsTriplette res = Chop(inP, input.length());

	if (res.error || res.segment->input.empty())
	{
		if (res.error)
			cout << "FAIL res.error" << res.error << "\n";
		if (!res.segment->input.empty())
			cout << "FAIL res.input" << res.segment->input.getCstr(buffer2, sizeof(buffer2)) << "\n";
	}

	const char * got = res.segment->input.getCstr(buffer2, sizeof(buffer2));

	if (got != want)
	{
		cout << "FAIL got: " << got << " but wanted " << want << "\n";
	}

	dest.reset();

	delete res.segment;
}

void testLeak()
{
	int testamount = 1000 * 1000 * 1000; // super huge takes a long time
	testamount = 1000 * 1000; 

	for (int pass = 0; pass < testamount; pass++)
	{
		string input = "a b c d e f g h a b c d e f g h a b c d e f g h ";

		const char *inP = input.c_str();

		ResultsTriplette res = Chop(inP, input.length());

		if (res.error || res.segment->input.empty())
		{
			if (res.error)
				cout << "FAIL res.error" << res.error << "\n";
			if (!res.segment->input.empty())
				cout << "FAIL res.input" << res.segment->input.getCstr(buffer2, sizeof(buffer2)) << "\n";
		}

		dest.reset();

		delete res.segment;
	}
}