//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "gtest/gtest.h"
#include <gleng.h>

struct TestComponent1 : public IComponent<TestComponent1> {
	int testValue;
};

struct TestComponent2 : public IComponent<TestComponent2> {
	float testFloat;
	uint64_t testBigint;
};

struct TestSharedComponent1 : public ISharedComponent<TestSharedComponent1> {
	char testArr[100];
	int testInt;
};

struct TestSharedComponent2 : public ISharedComponent<TestSharedComponent2> {
	std::string testString;
	int testInt;
};

struct TestSharedComponentWithDestructor : public ISharedComponent<TestSharedComponentWithDestructor> {
	std::string testString;
	int testInt;
	static int numDestructions;

	inline ~TestSharedComponentWithDestructor() {
		++numDestructions;
	}
};


struct TestEvent1 : public IEvent<TestEvent1> {
	Entity testEntity;
	int testInt;
};