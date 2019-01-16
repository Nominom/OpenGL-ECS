#include <stdio.h>
#include "..\include\entitymanager.h"

int main() {
	printf("Hello!\n");
	EntityManager manager;
	Entity entity1 = manager.CreateEntity();
	Entity entity2 = manager.CreateEntity();

	getchar();
}