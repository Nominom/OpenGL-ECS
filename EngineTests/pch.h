//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "gtest/gtest.h"
#include <gleng.h>

struct TestComponent1 : IComponent<TestComponent1> {
	int testValue;
};

struct TestComponent2 : IComponent<TestComponent2> {
	float testFloat;
	uint64_t testBigint;
};

struct TestSharedComponent1 : ISharedComponent<TestSharedComponent1> {
	char testArr[100];
	int testInt;
};

struct TestSharedComponent2 : ISharedComponent<TestSharedComponent2> {
	std::string testString;
	int testInt;
};