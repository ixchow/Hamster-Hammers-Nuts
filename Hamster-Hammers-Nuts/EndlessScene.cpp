#include "Assets.h"
#include "Scene.h"
#include "Game.h"
#include "Graphics.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <string>
#include <ctime>
#include <random>
#include <algorithm>
#include <iostream>

#define GROUND_WIDTH 15
#define GROUND_LENGTH 15
#define DROP_HEIGHT 15
#define SWING_FACTOR 1.3f
#define WALK_FACTOR 1.8f

namespace Hamster
{
	Object* EndlessScene::AddNut(glm::vec3 position, glm::quat rotation)
	{
		Object* object = new Object();
		object->velocity = glm::vec3(0.0f);
		object->transform.position = position;
		object->transform.rotation = rotation;
		object->transform.scale = glm::vec3(0.5f);
		object->height = 0.5f;
		object->length = 0.5f;
		object->width = 0.5f;
		object->mesh = Mesh(TOC::NUT_MESH);
		object->animated = false;
		nuts.push_back(object);
		return nuts.back();
	}

	Object* EndlessScene::AddLog(glm::vec3 position, glm::quat rotation)
	{
		Object* object = new Object();
		object->velocity = glm::vec3(0.0f);
		object->transform.position = position;
		object->transform.rotation = rotation;
		object->base_rotation = rotation;
		object->transform.scale = glm::vec3(1.0f);
		object->height = 1.0f;
		if (rotation.z <= 0.9f && rotation.z >= 0.4f) {
			object->length = 1.0f;
			object->width = 3.0f;
		}
		else {
			object->length = 3.0f;
			object->width = 1.0f;
		}
		object->mesh = Mesh(TOC::LOG_MESH);
		object->animated = false;
		logs.push_back(object);
		return logs.back();
	}

	EndlessScene::EndlessScene() : Scene()
	{
		// Center of hamster is not actual center
		hamster.transform.scale = glm::vec3(3.0f);
		hamster.transform.position.z = 0.0f;
		hamster.height = 3.0f;
		hamster.length = 1.0f;
		hamster.width = 1.0f;
		hamster.anim = Animation(TOC::HAMSTER_SKN, TOC::HAMSTER_STAND_ANIM);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_BODY_MESH);
		hamster.anim.mesh.emplace_back(TOC::HAMSTER_EYES_MESH);
		hamster.animated = true;
		direction = Direction::Down;

		ground.mesh = Mesh(TOC::GROUND_SPRING_MESH);
		target.mesh = Mesh(TOC::CIRCLE_MESH);
		target.transform.scale = glm::vec3(1.0f, 1.0f, 0.001f);

		ground.transform.scale = glm::vec3(0.5f);

		hawk.anim = Animation(TOC::ARMATURE_SKN, TOC::ARMATURE_DASH_ANIM);
		//hawk.anim = Animation(TOC::ARMATURE_SKN, TOC::ARMATURE_DASH_ANIM);
		hawk.anim.mesh.emplace_back(TOC::ARMATURE_BODY_MESH);
		hawk.animated = true;
		hawk.transform.scale = glm::vec3(3.0f);

		hawk.transform.position = glm::vec3(70.0f, 0.0f, -7.5f);
		hawk.height = 3.0f;
		hawk.length = 2.0f;
		hawk.width = 4.0f;


		camera.set(100.0f, 0.2f * M_PI, 1.0f * M_PI, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	bool EndlessScene::HandleInput()
	{
		if (Game::event.type == SDL_KEYDOWN && Game::event.key.keysym.sym == SDLK_SPACE && state <= State::Walking)
		{
			hamster.velocity = glm::vec3(0.0f);
			hamster.anim.Play(TOC::HAMSTER_SWING_ANIM, false, true, SWING_FACTOR);
			state = State::Swinging;
		}
		return true;
	}

	bool EndlessScene::Update()
	{
		static std::mt19937 mt_rand((unsigned int)time(NULL));

		current_time = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration<float>(current_time - previous_time).count();
		previous_time = current_time;
		if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		if (Game::KEYBD_STATE[SDL_SCANCODE_ESCAPE])
		{
			Game::NextScene(0);
			return false;
		}

		transition_time -= elapsed;
		if (transition_time < 0.0f) {
			level = (mt_rand() % 4)*3 + 1;
			transition_time = 20.0f;
			speed = 7.5f;
			windv = mt_rand() % 10;
			if (level < 4) {
				ground.mesh = Mesh(TOC::GROUND_SPRING_MESH);
			}
			else if (level < 7) {
				ground.mesh = Mesh(TOC::GROUND_SUMMER_MESH);
			}
			else if (level < 10) {
				ground.mesh = Mesh(TOC::GROUND_FALL_MESH);
			}
			else {
				ground.mesh = Mesh(TOC::GROUND_WINTER_MESH);
				speed = 5.0f;
			}
		}

		//if (stun == 0.0f && !on_ladder && !swinging)
		if (state <= State::Walking)
		{
			state = State::Walking;
			if (Game::KEYBD_STATE[SDL_SCANCODE_A] && !Game::KEYBD_STATE[SDL_SCANCODE_D])
			{
				if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = speed * 0.707107f;
					hamster.velocity.y = speed * 0.707107f;
				}
				else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = -speed * 0.707107f;
					hamster.velocity.y = speed * 0.707107f;
				}
				else {
					hamster.velocity.x = 0.0f;
					hamster.velocity.y = speed;
				}
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_A] && Game::KEYBD_STATE[SDL_SCANCODE_D])
			{
				if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = speed * 0.707107f;
					hamster.velocity.y = -speed * 0.707107f;
				}
				else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
					hamster.velocity.x = -speed * 0.707107f;
					hamster.velocity.y = -speed * 0.707107f;
				}
				else {
					hamster.velocity.x = 0.0f;
					hamster.velocity.y = -speed;
				}
			}
			else if (Game::KEYBD_STATE[SDL_SCANCODE_W] && !Game::KEYBD_STATE[SDL_SCANCODE_S]) {
				hamster.velocity.x = speed;
				hamster.velocity.y = 0.0f;
			}
			else if (!Game::KEYBD_STATE[SDL_SCANCODE_W] && Game::KEYBD_STATE[SDL_SCANCODE_S]) {
				hamster.velocity.x = -speed;
				hamster.velocity.y = 0.0f;
			}
			else {
				state = State::Idle;
				hamster.velocity.x = 0.0f;
				hamster.velocity.y = 0.0f;
			}
		}

		if (state <= State::Walking)
		{
			if (hamster.velocity.x > 0.0f) {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::RightUp;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::LeftUp;
				}
				else {
					direction = Direction::Up;
				}
			}
			else if (hamster.velocity.x < 0.0f) {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::RightDown;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::LeftDown;
				}
				else {
					direction = Direction::Down;
				}
			}
			else {
				if (hamster.velocity.y < 0.0f) {
					direction = Direction::Right;
				}
				else if (hamster.velocity.y > 0.0f) {
					direction = Direction::Left;
				}
			}
		}


		float x1 = hamster.transform.position.x + elapsed * hamster.velocity.x;
		float y1 = hamster.transform.position.y + elapsed * hamster.velocity.y;
		float z1 = hamster.transform.position.z;

		// 
		//if (x1 > GROUND_LENGTH - hamster.length && stun == 0.0f && !swinging && ladder.transform.position.z == 20.0f)

		if (abs(hamster.transform.position.x) - 2.0f * hamster.length > GROUND_LENGTH ||
			abs(hamster.transform.position.y) - 2.0f * hamster.width > GROUND_WIDTH)
		{
			state = State::Falling0;
			//if (grabbed)
			//	grabbed = false;
			//on_ladder = true;
			//transition = true;
			score = std::max(0, score - 5);
			hamster.velocity.x = 0.0f;
			hamster.velocity.y = 0.0f;
			hamster.velocity.z -= elapsed * gravity;
		}

		//if (stun != 0.0f)
		if (state == State::Stunned)
		{
			stun -= elapsed;
			RotateObject(&hamster, 540.0f * elapsed, glm::vec3(0.0f, 0.0f, 1.0f));
			if (stun < 0.0f)
				state = State::Idle;
			//stun = 0.0f;
		}

		//if (stun != 0.0f) {
		//	stun -= elapsed;
		//	if (stun < 0.0f)
		//		stun = 0.0f;
		//}
		if (state != State::OnLadder0 && state != State::OnLadder1 && state != State::OnLadder2) {
			for (auto log : logs)
			{
				if (abs(log->transform.position.z + elapsed * log->velocity.z - z1) <= hamster.height + log->height &&
					log->transform.position.z > log->height &&
					log->velocity.z < 0.0f)
				{
					if (abs(log->transform.position.x - hamster.transform.position.x) < hamster.length + log->length &&
						abs(log->transform.position.y - hamster.transform.position.y) < hamster.width + log->width)
					{
						state = State::Stunned;
						stun = 1.0f;
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (log->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0;
						if (log->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
						if (score > 0)
							score--;
						break;
					}
				}
			}
			for (auto nut : nuts)
			{
				if (abs(abs(nut->transform.position.z + elapsed * nut->velocity.z - z1) - hamster.height - nut->height) < 0.25f &&
					nut->transform.position.z > nut->height && nut->velocity.z < 0.0f)
				{
					if (abs(nut->transform.position.x - hamster.transform.position.x) < hamster.length + nut->length &&
						abs(nut->transform.position.y - hamster.transform.position.y) < hamster.width + nut->width)
					{
						state = State::Stunned;
						stun = 1.0f;
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (nut->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0;
						if (nut->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
						if (score > 0)
							score--;
						break;
					}
				}
			}
			for (auto log : logs) {
				if (abs(log->transform.position.z - z1) <= hamster.height + log->height && state != State::Stunned) {
					if (abs(log->transform.position.x - x1) < hamster.length + log->length && abs(log->transform.position.y - y1) < hamster.width + log->width) {
						hamster.velocity.x = log->velocity.x;
						hamster.velocity.y = log->velocity.y;
					}
				}
				if (abs(log->transform.position.z - z1) <= hamster.height + log->height && state == State::Stunned) {
					if (abs(log->transform.position.x - x1) < hamster.length + log->length && abs(log->transform.position.y - y1) < hamster.width + log->width) {
						hamster.velocity.x = 10.0f;
						hamster.velocity.y = 10.0f;
						if (log->transform.position.y - hamster.transform.position.y > 0.0f)
							hamster.velocity.y = -10.0f;
						if (log->transform.position.x - hamster.transform.position.x > 0.0f)
							hamster.velocity.x = -10.0f;
					}
				}
			}
		}

		// Update object animations
		hamster.anim.Update(elapsed);
		hawk.anim.Update(elapsed);

		//if (!swinging)
		//if (state != State::Swinging)
		if (state == State::Idle)
		{
			hamster.anim.Play(TOC::HAMSTER_STAND_ANIM);
		}
		else if (state == State::Walking)
		{
			hamster.anim.Play(TOC::HAMSTER_WALK_ANIM, true, true, WALK_FACTOR);
		}

		next_drop -= elapsed;
		if (next_drop <= 0.0f) {
			next_drop = drop_interval;
			int drop_type = mt_rand() % (level / 4 + 1);
			glm::vec3 pos =
				glm::vec3((float)(mt_rand() % (2 * (GROUND_LENGTH - 3))) - ((float)GROUND_LENGTH - 3.0f),
				(float)(mt_rand() % (2 * (GROUND_WIDTH - 3))) - ((float)GROUND_WIDTH - 3.0f), 50.0f);
			bool can_drop = true;
			for (auto log : logs) {
				if (abs(pos.x - log->transform.position.x) <= 4.0f || abs(pos.y - log->transform.position.y) <= 4.0f) {
					can_drop = false;
					break;
				}
			}
			for (auto nut : nuts) {
				if (abs(pos.x - nut->transform.position.x) <= 1.0f || abs(pos.y - nut->transform.position.y) <= 1.0f) {
					can_drop = false;
					break;
				}
			}
			if (can_drop) {
				if (drop_type == 0) {
					AddNut(pos,
						normalize(glm::quat(mt_rand() % 10, mt_rand() % 10, mt_rand() % 10, mt_rand() % 10)));
				}
				else {
					int rotation = mt_rand() % 4;
					glm::quat quart = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
					quart = glm::rotate(quart, (float)M_PI*(float)rotation / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
					AddLog(pos, quart);

				}
			}
		}

		for (auto it = logs.begin(); it != logs.end();) {
			auto log = *it;
			if (log->transform.position.z > log->height) {
				log->velocity.z -= elapsed*gravity;
			}
			if (abs(log->transform.position.x) - log->length > GROUND_LENGTH || abs(log->transform.position.y) - log->width > GROUND_WIDTH) {
				log->velocity.z -= elapsed*gravity;
			}
			if (abs(log->transform.position.x) - log->length < GROUND_LENGTH && abs(log->transform.position.y) - log->width < GROUND_WIDTH) {
				if (log->transform.position.z <= log->height && log->velocity.z < 0.0f) {
					log->velocity.z = -log->velocity.z / 4.0f;
					if (abs(log->velocity.z) < 1.0f) {
						log->transform.position.z = log->height;
						log->velocity.z = 0.0f;
					}
				}
			}
			if (log->transform.position.z < -10.0f)
				it = logs.erase(it);
			else
				it++;
		}
		for (auto it = nuts.begin(); it != nuts.end();) {
			auto nut = *it;
			if (nut->transform.position.z > nut->height) {
				nut->velocity.z -= elapsed*gravity;
			}
			if (abs(nut->transform.position.x) - nut->length > GROUND_LENGTH || abs(nut->transform.position.y) - nut->width > GROUND_WIDTH) {
				nut->velocity.z -= elapsed*gravity;
			}
			if (abs(nut->transform.position.x) - nut->length < GROUND_LENGTH && abs(nut->transform.position.y) - nut->width < GROUND_WIDTH) {
				if (nut->transform.position.z <= nut->height && nut->velocity.z < 0.0f) {
					nut->velocity.z = -nut->velocity.z / 4.0f;
					if (abs(nut->velocity.z) < 1.0f) {
						nut->transform.position.z = nut->height;
						nut->velocity.z = 0.0f;
					}
				}
			}
			if (nut->transform.position.z < -10.0f)
				it = nuts.erase(it);
			else
				it++;
		}
		//wind
		if (level >= 4) {
			if (windt != 0.0f) {
				windt -= elapsed;
				if (windt < 0.0f) {
					windt = 0.0f;
					windyv = 0.0f;
					windxv = 0.0f;
				}
			}
			else {
				int dir = mt_rand() % 6;
				windt = 10.0f;
				if (dir < 3) {
					windyv = windv*(dir - 1);
					windxv = 0.0f;
				}
				else {
					windxv = windv*(dir - 4);
					windyv = 0.0f;
				}
			}
		}
		//Log rolling not fully working
		for (auto log : logs) {
			log->velocity.y = windyv;
			log->velocity.x = windxv;
			if (log->length != 1.0f && log->base_rotation.z == 1.0f) {
				RotateObject(log, (float)M_PI / 4.0f*elapsed*windyv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length != 1.0f && log->base_rotation.w == 1.0f) {
				RotateObject(log, (float)-M_PI / 4.0f*elapsed*windyv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length == 1.0f && log->base_rotation.w > 0.0f) {
				RotateObject(log, (float)M_PI / 4.0f*elapsed*windxv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			if (log->length == 1.0f && log->base_rotation.w < 0.0f) {
				RotateObject(log, (float)-M_PI / 4.0f*elapsed*windxv, glm::vec3(1.0f, 0.0f, 0.0f));
			}
		}
		for (auto nut : nuts) {
			nut->velocity.y = windyv;
			nut->velocity.x = windxv;
		}
		if (level >= 7) {
			if (hawk.transform.position.y > 60.0f) {
				hawk.velocity.y = -10.0f;
				hawk.transform.position.x = mt_rand() % (2 * (GROUND_LENGTH - 3)) - GROUND_LENGTH + 3;
			}
			if (hawk.transform.position.y < -60.0f) {
				hawk.velocity.y = 10.0f;
				hawk.transform.position.x = mt_rand() % (2 * (GROUND_LENGTH - 3)) - GROUND_LENGTH + 3;
			}
			if (abs(hawk.transform.position.x - hamster.transform.position.x) < 2.0f && abs(hawk.transform.position.y - hamster.transform.position.y) < 2.0f &&
				abs(hamster.transform.position.x) - 2.0f * hamster.length < GROUND_LENGTH &&
				abs(hamster.transform.position.y) - 2.0f * hamster.width < GROUND_WIDTH) {
				//grabbed = true;
				state = State::Hawked;
			}

			if (state == State::Hawked) {
				hamster.velocity = hawk.velocity;
			}
		}

		hawk.transform.position += elapsed*hawk.velocity;
		hamster.transform.position += elapsed*hamster.velocity;
		
		if (hamster.transform.position.z < -30.0f && state == State::Falling0 && hamster.velocity.z < 0.0f) {
			logs.clear();
			nuts.clear();
			//END GAME HERE SOMEHOW
		}
		
		for (auto nut : nuts) {
			nut->transform.position += nut->velocity*elapsed;
		}
		for (auto log : logs) {
			log->transform.position += log->velocity*elapsed;
		}

		if (state == State::Swinging && hamster.anim.frame_number == 15) {

			for (auto it = nuts.begin(); it != nuts.end(); it++) {
				auto nut = *it;
				if (nut->velocity.z == 0.0f) {
					if (direction == Direction::Up && abs(hamster.transform.position.x - nut->transform.position.x + 2.0f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::Down && abs(hamster.transform.position.x - nut->transform.position.x - 2.0f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::Left && abs(hamster.transform.position.x - nut->transform.position.x) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y + 2.0f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::Right && abs(hamster.transform.position.x - nut->transform.position.x) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y - 2.0f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::LeftUp && abs(hamster.transform.position.x - nut->transform.position.x + 2.0f*0.707107f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y + 2.0f*0.707107f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::LeftDown && abs(hamster.transform.position.x - nut->transform.position.x - 2.0f*0.707107f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y + 2.0f*0.707107f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::RightUp && abs(hamster.transform.position.x - nut->transform.position.x + 2.0f*0.707107f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y - 2.0f*0.707107f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
					if (direction == Direction::RightDown && abs(hamster.transform.position.x - nut->transform.position.x - 2.0f*0.707107f) <= 1.0f &&
						abs(hamster.transform.position.y - nut->transform.position.y - 2.0f*0.707107f) <= 1.0f) {
						it = nuts.erase(it);
						score++;
						break;
					}
				}
			}
		}

		if (state == State::Swinging && hamster.anim.state == AnimationState::FINISHED)
		{
			state = State::Idle;
			//swinging = false;
		}
		//if (state != State::OnLadder0 && state != State::OnLadder1 && state != State::OnLadder2)
		if (state != State::Stunned)
			RotateDirection(&hamster, direction);
		if (hawk.velocity.y < 0.0f) {
			RotateDirection(&hawk, Direction::Right);
		}
		else {
			RotateDirection(&hawk, Direction::Left);
		}
		return true;
	}

	void EndlessScene::Render()
	{
		// camera stuff
		glm::mat4 world_to_camera = camera.transform.make_world_to_local();
		glm::mat4 world_to_clip = camera.make_projection() * world_to_camera;
		glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 to_light = glm::normalize(glm::mat3(world_to_camera) * light_pos);

		// compute model view projection from the light's point of view
		glm::mat4 light_projection = glm::ortho<float>(-17.0f, 17.0f, -17.0f, 17.0f, 0.0f, 100.0f);
		glm::mat4 light_view = glm::lookAt(50.0f * light_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 world_to_light = light_projection * light_view;

		// set world transforms (ALL CONSTANT RIGHT NOW)
		Graphics::WorldTransforms(world_to_camera, world_to_clip, world_to_light);

		// shadow map
		Graphics::BeginShadow();
		for (auto it = nuts.begin(); it != nuts.end(); it++)
			Graphics::RenderShadow(**it);
		for (auto it = logs.begin(); it != logs.end(); it++)
			Graphics::RenderShadow(**it);
		Graphics::RenderShadow(hamster);
		Graphics::RenderShadow(hawk);

		// scene
		Graphics::BeginScene(to_light);
		for (auto it = nuts.begin(); it != nuts.end(); it++)
			Graphics::RenderScene(**it);
		for (auto it = logs.begin(); it != logs.end(); it++)
			Graphics::RenderScene(**it);
		Graphics::RenderScene(hamster);
		Graphics::RenderScene(ground);
		Graphics::RenderScene(hawk);
		if (state <= State::Swinging)
		{
			switch (direction)
			{
			case Direction::Left:
				target.transform.position = hamster.transform.position + glm::vec3(0.0f, 2.0f, 0.0f);
				break;
			case Direction::Right:
				target.transform.position = hamster.transform.position + glm::vec3(0.0f, -2.0f, 0.0f);
				break;
			case Direction::Down:
				target.transform.position = hamster.transform.position + glm::vec3(-2.0f, 0.0f, 0.0f);
				break;
			case Direction::Up:
				target.transform.position = hamster.transform.position + glm::vec3(2.0f, 0.0f, 0.0f);
				break;
			case Direction::LeftDown:
				target.transform.position = hamster.transform.position + glm::vec3(-2.0f * 0.707107f, 2.0f * 0.707107f, 0.0f);
				break;
			case Direction::LeftUp:
				target.transform.position = hamster.transform.position + glm::vec3(2.0f * 0.707107f, 2.0f * 0.707107f, 0.0f);
				break;
			case Direction::RightDown:
				target.transform.position = hamster.transform.position + glm::vec3(-2.0f * 0.707107f, -2.0f * 0.707107f, 0.0f);
				break;
			case Direction::RightUp:
				target.transform.position = hamster.transform.position + glm::vec3(2.0f * 0.707107f, -2.0f * 0.707107f, 0.0f);
				break;
			}
			Graphics::RenderScene(target, 0.75f);
			target.transform.position = hamster.transform.position;
			//target.transform.scale = glm::vec3(0.4f, 0.4f, 0.001f);
			Graphics::RenderScene(target, 0.75f);
			//target.transform.scale = glm::vec3(1.0f, 1.0f, 0.001f);
		}

		// background
		Graphics::BeginSprite();
		Graphics::RenderSprite(TOC::SKY_PNG, glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f));

		// actual scene
		Graphics::CompositeScene();

		// ui
		Graphics::BeginSprite();
		// draw acorn
		// draw numbers depending on # of digits of max score
		
		// draw first number
		if (score < 10) // draw 0
		{

		}
		else
		{

		}
		// draw /
		// draw second number
		

		Graphics::Present();
	}
}