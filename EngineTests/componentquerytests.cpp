#include "pch.h"


TEST(ComponentQueries, Build) {

	ComponentQuery query =
		ComponentQueryBuilder().Include<TestComponent1, TestSharedComponent1, 
		Optional<TestSharedComponentWithDestructor>>()
		.Exclude<TestSharedComponent2>().Exclude<TestComponent2>().Build();

	ASSERT_EQ(query.types.size(), 4);
	ASSERT_EQ(query.types[0], TestComponent1::ComponentTypeID);
	ASSERT_EQ(query.types[1], TestComponent2::ComponentTypeID);
	ASSERT_EQ(query.types[2], TestSharedComponent1::ComponentTypeID);
	ASSERT_EQ(query.types[3], TestSharedComponent2::ComponentTypeID);

}

TEST(ComponentQueries, Match) {

	TestSharedComponent1 shared1;
	TestSharedComponent2 shared2;

	EntityArchetype archetype_all =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>())
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	EntityArchetype archetype_components =
		EntityArchetype(ComponentType::Get<TestComponent1>())
		.AddComponent(ComponentType::Get<TestComponent2>());

	EntityArchetype archetype_sharedcomponents =
		EntityArchetype()
		.AddSharedComponent(&shared1)
		.AddSharedComponent(&shared2);

	ComponentQuery filter1 = ComponentQueryBuilder().Include<TestComponent1, TestComponent2,
		TestSharedComponent1, TestSharedComponent2>().Build();

	ComponentQuery filter2 = ComponentQueryBuilder().Include<TestComponent1>().Build();

	ComponentQuery filter3 = ComponentQueryBuilder().Include<TestComponent2>()
		.Exclude<TestSharedComponent1>().Build();

	ComponentQuery filter4 = ComponentQueryBuilder().Include<TestSharedComponent1>()
		.Exclude<TestComponent1>().Build();

	ASSERT_TRUE(filter1.Matches(archetype_all));
	ASSERT_FALSE(filter1.Matches(archetype_components));
	ASSERT_FALSE(filter1.Matches(archetype_sharedcomponents));

	ASSERT_TRUE(filter2.Matches(archetype_all));
	ASSERT_TRUE(filter2.Matches(archetype_components));
	ASSERT_FALSE(filter2.Matches(archetype_sharedcomponents));

	ASSERT_FALSE(filter3.Matches(archetype_all));
	ASSERT_TRUE(filter3.Matches(archetype_components));
	ASSERT_FALSE(filter3.Matches(archetype_sharedcomponents));

	ASSERT_FALSE(filter4.Matches(archetype_all));
	ASSERT_FALSE(filter4.Matches(archetype_components));
	ASSERT_TRUE(filter4.Matches(archetype_sharedcomponents));
}