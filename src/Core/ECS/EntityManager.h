//
// Created by Coby Jennings on 2026/10/25.
//

#ifndef GAMECUBEENGINE_ENTITYROOT_H
#define GAMECUBEENGINE_ENTITYROOT_H

#include "../GenericTypes.h"

// This is probably fucking massive for the GC
#define MAX_ENTITIES 65535
#define Entity uint32
#define Signature uint32

/*
 * https://austinmorlan.com/posts/entity_component_system/#the-signature
 *
 * ^ however I think we should replace signatures with a non crypto hashing function
 * that has a low collision rate, because bitwise operations in
*/

class EntityManager
{
public:
	EntityManager();
	Entity CreateEntity();
	void DestroyEntity(Entity Ent);
	void SetSignature(Entity Ent, Signature Sig);
	void GetSignature(Entity ENt, Signature Sig);

private:
	uint32 LivingEntityCount{0};
};

#endif // GAMECUBEENGINE_ENTITYROOT_H