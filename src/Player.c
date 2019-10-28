#include "Player.h"
#include "gfc_input.h"
#include "gf3d_camera.h"
#include "gf3d_space.h"
#include "collectable.h"
#include "simple_logger.h"

#define MOVE_SPEED 0.1
#define ROTATE_SPEED 0.05
#define JUMP_HEIGHT 3.0
static Entity* player = NULL;

void player_think(Entity* self);
void player_update(Entity* self);
void player_touch(Entity* self, Entity* other);


Entity* Player_New() {

	player = gf3d_entity_new();
	player->tag = "player";
	player->name = "player";
	//player->model = gf3d_model_load("sphere");
	player->model = gf3d_model_load_animated("sphere_anim", "sphere_anim", 1, 23);
	gfc_matrix_identity(player->modelMat);
	player->position = vector3d(0, 0, 0);
	player->rotation = vector3d(0, 0, 0);
	player->shape = gf3d_shape_sphere(1, player->position);
	player->rayf = gf3d_ray_set(player->position, vector3d(0, 0, 1));
	gf3d_body_set(
		&player->body,
		0,
		0,
		0,
		1,
		player->position,
		vector3d(0, 0, 0),
		1,
		0,
		0,
		&player->shape,
		NULL,
		player);
	player->shape.data = player;
	player->think = player_think;
	player->update = player_update;
	player->grounded = 0;
	return player;
}

void player_think(Entity* self) {

	if (gfc_input_key_down("a")) {

		/*gfc_matrix_rotate(
			player->modelMat,
			player->modelMat,
			0.002,
			vector3d(0, 0, -1)
		);*/

		player->velocity.x = MOVE_SPEED;
		//gf3d_entity_move(self, vector3d(MOVE_SPEED, 0, 0));

	}

	if (gfc_input_key_down("d")) {

		/*gfc_matrix_rotate(
			player->modelMat,
			player->modelMat,
			0.002,
			vector3d(0, 0, -1)
		);*/
		player->velocity.x = -MOVE_SPEED;
		//gf3d_entity_move(self, vector3d(-MOVE_SPEED, 0, 0));

	}

	if (gfc_input_key_down("w")) {

		player->velocity.y = -MOVE_SPEED;
		//gf3d_entity_move(self, vector3d(0, -MOVE_SPEED, 0));

	}

	if (gfc_input_key_down("s")) {

		/*gfc_matrix_rotate(
			player->modelMat,
			player->modelMat,
			0.002,
			vector3d(0, 0, -1)
		);*/

		player->velocity.y = MOVE_SPEED;

		//gf3d_entity_move(self, vector3d(0, MOVE_SPEED, 0));

	}



	if (gfc_input_key_down("q")) {

		self->rotation.z -= ROTATE_SPEED;


	}

	if (gfc_input_key_down("e")) {

		self->rotation.z += ROTATE_SPEED;

		//gfc_matrix_translate(player->modelMat, vector3d(0, 0.01, 0));

		
	}

	if (gfc_input_key_down(" ") && player->grounded) {
		player->frame += 1;
		if (player->frame > 19) {
			player->frame = 19;
		}
	}

	if (gfc_input_key_released(" ")) {
		player->frame = 0;
	}

	if (gfc_input_key_pressed(" ") && player->grounded) {
		player->jumpTime = 2.0f;
	}
	
	//slog("total entities %i", get_entity_size());
	//gf3d_entity_move(self, vector3d(0, 0, -9));
	if (player->jumpTime > 0.0f) {
		player->jumpTime -= 0.1;
		player->velocity.z += GRAVITY;
	}
	else {
		player->jumpTime = 0.0f;
		player->velocity.z -= GRAVITY;
	}
	
	//slog("player frame %i", player->frame);
}

void player_set_collectable(Collectable* collectable) {

	switch (collectable->color){

	case Collectable_Red:
		player->red++;
		break;
	case Collectable_Blue:
		player->blue++;
		break;
	case Collectable_Yellow:
		player->yellow++;
		break;
	}

}

void player_update(Entity* self) {

	Vector3D oldPos = self->shape.s.sp.point.pos;
	Uint8 collided = 0;
	//gf3d_entity_move(self, self->velocity);
	
	self->shape.s.sp.point.pos.x += self->velocity.x;
	self->shape.s.sp.point.pos.y += self->velocity.y;
	self->shape.s.sp.point.pos.z += self->velocity.z;

	for (int i = 0; i < get_entity_size(); i++) {

		Body* ent_body = get_entity_bodies(i);
		Entity* other = (Entity*)ent_body->shape->data;
		//slog("Testing against %s", other->name);

		if (!ent_body->shape) {
			continue;
		}

		if (&player->body == ent_body) {
			continue;
		}

		if (!other->_inuse) {
			continue;
		}
		if (gf3d_body_body_collide(&player->body, ent_body)) {
			Entity* other = ent_body->shape->data;
			if (strcmp(other->tag, "ground") == 0) {
				player->grounded = 1;
				collided = 1;
			}
			else if (strcmp(other->tag, "collectable") == 0) {
				Collectable* collectable = other->data;

				player_set_collectable(collectable);
				slog("Colliding with collectable %s", &collectable->color);
				other->_inuse = 0;
			}

		}
	}

	if (collided) {
		self->shape.s.sp.point.pos = oldPos;
	}
	else {
		gf3d_entity_move(self, self->velocity);
		player->grounded = 0;
	}
	//player->body.position = player->position;
	gfc_matrix_rotate(
		self->modelMat,
		self->modelMat,
		self->rotation.z,
		vector3d(0, 0, 1)
	);
	slog("Player collectables Red: %i, Blue: %i, Yellow: %i", player->red, player->blue, player->yellow);
	//slog("player roation in degrees - %f", radians_to_degrees(self->rotation.z));
	player->rayf = FromPoints(self->position, self->rayf.dir);
	//slog("Ray dir - %f, %f, %f", self->rayf.dir.x, self->rayf.dir.y, self->rayf.dir.z);
	player->velocity = vector3d(0, 0, 0);
}