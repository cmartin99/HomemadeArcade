

ALWAYS_INLINE float EntityRadius(int32 mass)
{
	return sqrt(mass / g_PI) + 1; //√(1200/(4π)) = r
}

ALWAYS_INLINE float EntityRadius(const Mass* entity)
{
	return sqrt(entity->mass / g_PI) + 1; //√(1200/(4π)) = r
}

ALWAYS_INLINE void MassSet(Entity* entity, float mass)
{
	entity->mass_change_orig = entity->mass = 1;
	entity->mass_target = mass;
	if (entity->mass_change_time_orig == 0)
		entity->mass_change_time_orig = GetAppState()->sim.total_seconds;
}

ALWAYS_INLINE void MassChange(Entity* entity, float mass)
{
	entity->mass_change_orig = entity->mass;
	entity->mass_target += mass;
	if (entity->mass_change_time_orig == 0)
		entity->mass_change_time_orig = GetAppState()->sim.total_seconds;
}

ALWAYS_INLINE float MergeDelayPrimary(const Sim* sim, float mass)
{
	//return tdLerp(20.f, 300.f, (float)mass / world->max_mass_per_piece);
	return 30.f + mass * 0.005f;
}

ALWAYS_INLINE float MergeDelaySecondary(const Sim* sim, float mass)
{
	return mass * 0.025f;
}

ALWAYS_INLINE Entity* FirstPlayerCell(const Sim* sim, const Player* player)
{
	return sim->player_cells.ptr + (player - sim->players.ptr) * sim->max_player_cells;
}

ALWAYS_INLINE Entity* FirstPlayerCellDisabled(const Sim* sim, const Player* player, Entity* from)
{
	Entity* cell = (from == nullptr) ? FirstPlayerCell(sim, player) : from + 1;
	Entity* cell_end = FirstPlayerCell(sim, player) + sim->max_player_cells;
	while (cell < cell_end)
	{
		if (!cell->enabled) return cell;
		++cell;
	}
	return nullptr;
}

ALWAYS_INLINE Player* PlayerFromCell(const Sim* sim, const Entity* cell)
{
	return sim->players.ptr + (cell - sim->player_cells.ptr) / sim->max_player_cells;
}

ALWAYS_INLINE float PlayerTotalMass(const Sim* sim, const Player* player)
{
	Entity* cell = FirstPlayerCell(sim, player);
	Entity* cell_end = cell + sim->max_player_cells;
	float result = 0;
	while (cell < cell_end)
	{
		if (cell->enabled) result += cell->mass_target; else break;
		++cell;
	}
	return result;
}

ALWAYS_INLINE int32 PlayerCellCount(const Sim* sim, const Player* player)
{
	int32 result = 0;
	Entity* cell = FirstPlayerCell(sim, player);
	Entity* cell_end = cell + sim->max_player_cells;
	while (cell < cell_end)
	{
		if (cell->enabled)
			++result;
		else
			break;
		++cell;
	}
	return result;
}

ALWAYS_INLINE bool IsCellMoving(const Mass* cell)
{
	return cell->vel.x || cell->vel.y;
}

ALWAYS_INLINE Vector2 MousePos(const Player* player)
{
	assert(player);
	
	if (IsBot(player) || !player->gamer->movement_paused)
		return Vector2(player->state_data.mouse_pos_x, player->state_data.mouse_pos_y);
	else
		return player->center;
}

Vector2 RandomWorldPosition(Sim* sim, Vector2 origin, float max_dist_from_orig, float dist_from_edge = 0)
{
	TIMED_BLOCK(RandomWorldPosition);
	float diameter = max_dist_from_orig + max_dist_from_orig;
	Vector2 pos;
	retry:
	pos.x = (int16)tdRandomNext(sim->rng, diameter) - max_dist_from_orig;
	pos.y = (int16)tdRandomNext(sim->rng, diameter) - max_dist_from_orig;
	if (length(pos) >= max_dist_from_orig) goto retry;
	if (length(pos + origin) >= sim->max_world_size / 2 - dist_from_edge) goto retry;
	return pos + origin;
}

ALWAYS_INLINE Vector2 RandomWorldPosition(Sim* sim, float dist_from_edge = 0)
{
	return RandomWorldPosition(sim, Vector2(0,0), sim->world_size / 2, dist_from_edge);
}

Vector2 RandomWorldPositionAwayFromEntities(Sim* sim)
{
	TIMED_BLOCK(RandomWorldPositionAwayFromEntities);
	Vector2 pos;
	int32 half_world_size = sim->world_size / 2 - 10;
	//half_world_size *= half_world_size;
	int32 tries = 0;
	retry:
	pos.x = (int16)tdRandomNext(sim->rng, sim->world_size - 20) - half_world_size;
	pos.y = (int16)tdRandomNext(sim->rng, sim->world_size - 20) - half_world_size;
	if (length(pos) >= half_world_size) goto retry;

	float radius, space = ++tries < 50 ? 40 : 20;

	Entity* cell, *cell_end;
	Player* player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			while (cell < cell_end)
			{
				if (cell->enabled)
				{
					radius = EntityRadius(cell);
					if (distance2(pos, cell->pos) <= (radius + space) * (radius + space))
						goto retry;
				}
				else
					break;
				++cell;
			}
		}
		++player;
	}

	radius = EntityRadius(g_virus_mass);
	float d = (radius + space) * (radius + space);
	Mass* mass = sim->viruses.ptr;
	for (int i = 0; i < sim->viruses.count; ++i, ++mass)
	{
		if (mass->enabled)
			if (distance2(pos, mass->pos) <= d)
				goto retry;
	}

	Entity* spawner = sim->spawners.ptr;
	for (int i = 0; i < sim->spawners.count; ++i, ++spawner)
	{
		if (spawner->enabled)
		{
			radius = EntityRadius(spawner);
			d = (radius + space) * (radius + space);
			if (distance2(pos, spawner->pos) <= d)
				goto retry;
		}
	}

	return pos;
}

Mass* GetNextFreeShot(Sim* sim)
{
	TIMED_BLOCK(GetNextFreeShot);
	for (int32 i = 0; i < sim->shots.count; ++i)
		if (!sim->shots[i].enabled) return sim->shots.ptr + i;

	return sim->shots.count < sim->shots.cap ? tdArrayPush(sim->shots) : nullptr;
}

Entity* GetRandomEmittingSpawner(Sim* sim)
{
	TIMED_BLOCK(GetRandomEmittingSpawner);
	Entity* result = nullptr;
	if (sim->spawners_emitting.count > 0)
	{
		int i = tdRandomNext(sim->rng, sim->spawners_emitting.count);
		result = sim->spawners.ptr + sim->spawners_emitting[i];

		if (result->mass_target <= g_spawner_mass)
		{
			tdArrayRemoveAt(sim->spawners_emitting, i);
			result = nullptr;
		}
	}
	return result;
}

void DisableCell(Sim* sim, Entity* cell)
{
	///TODO not thread safe
	Player* player = PlayerFromCell(sim, cell);
	cell->state = 0;
	Entity* cell_start = FirstPlayerCell(sim, player);
	Entity* cell_end = cell_start + sim->max_player_cells;
	if (cell < cell_end - 1 && (cell + 1)->enabled)
	{
		memmove(cell, cell + 1, sizeof(Entity) * (sim->max_player_cells - (cell - cell_start + 1)));
		(cell_end - 1)->state = 0;
		while (cell < cell_end && cell->enabled)
		{ 
			cell->state = 2; // flag state as refresh
			++cell;
		}
	}
	else
	{
		if (cell == cell_start)
		{
			--sim->player_count;
			player->type = 0;
			if (!player->gamer) 
				AddBot(sim, player);
			else if ((player - sim->players.ptr) == sim->players.count - 1)
				--sim->players.count;
		}
	}
}

void AddMovingAgar(Sim* sim, const Entity* spawner, Agar* agar)
{
	float r = EntityRadius(spawner);
	agar->color_id = (uint8)tdRandomNext(sim->rng, agar_color_count - 4) + 4;
	//retry:
	float x = (float)(tdRandomNextDouble(sim->rng) - 0.5);
	float y = (float)(tdRandomNextDouble(sim->rng) - 0.5);
	Vector2 dir = normalize(Vector2(x, y));
	Vector2 pos = spawner->pos + dir * (r - 2);
	Vector2 vel = dir * (8.f + (float)(tdRandomNextDouble(sim->rng) * 14.0));
	Vector2 dest = pos + vel * 0.75f;
	agar->x = (int16)dest.x;
	agar->y = (int16)dest.y;
}

void PlayerSplit(Sim* sim, const Player* player)
{
	int32 cell_count = PlayerCellCount(sim, player);
	int new_cell_count = min(cell_count, sim->max_player_cells - cell_count);
	if (new_cell_count > 0)
	{
		Vector2 mouse_pos(player->state_data.mouse_pos_x, player->state_data.mouse_pos_y);
		Entity* cell = FirstPlayerCell(sim, player);
		Entity* cell2 = cell + cell_count;
		for (int j = 0; j < new_cell_count; ++j)
		{
			if (cell->enabled && cell->mass_target >= 16)
			{
				float m = cell->mass_target * 0.5f;
				MassChange(cell, -m);
				cell2->state = 2; // new cell (state refresh)
				MassSet(cell2, m);
				cell2->pos.x = cell->pos.x;
				cell2->pos.y = cell->pos.y;
				float merge_delay = MergeDelayPrimary(sim, cell->mass_target);
				cell->merge_time = cell2->merge_time = sim->total_seconds + merge_delay;
				Vector2 delta = mouse_pos - cell->pos;
				if (length2(delta) > 0.f) 
					cell2->vel = cell->vel * 0.5f + normalize(delta) * g_split_speed;
				++cell2;
			}
			++cell;
		}
		sim->next_socket_write_time	= 0;
	}
}

bool PlayerSplitCell(Sim* sim, const Player* player, Entity* cell, Vector2 mouse_pos)
{
	Entity* cell1 = FirstPlayerCell(sim, player);
	Entity* cell2 = cell1 + sim->max_player_cells;
	while (cell1 < cell2)
	{
		if (!cell1->enabled)
		{
			float m = cell->mass_target * 0.5f;
			MassChange(cell, -m);
			cell1->state = 2; // new cell (state refresh)
			MassSet(cell1, m);
			cell1->pos.x = cell->pos.x;
			cell1->pos.y = cell->pos.y;
			float merge_delay = MergeDelayPrimary(sim, cell->mass_target);
			cell->merge_time = cell1->merge_time = sim->total_seconds + merge_delay;
			Vector2 delta = mouse_pos - cell->pos;
			if (length2(delta) > 0.f) 
				cell1->vel = cell->vel * 0.5f + normalize(delta) * g_split_speed;
			return true;
		}
		++cell1;
		sim->next_socket_write_time	= 0;
	}
	return false;
}

void PlayerShoot(Sim* sim, Player* player)
{
	player->next_shot_time = sim->total_seconds + g_shot_repeat_speed;
	float shot_speed = 200;
	Vector2 mouse_pos(player->state_data.mouse_pos_x, player->state_data.mouse_pos_y);
	Entity* cell = FirstPlayerCell(sim, player);
	Entity* cell_end = cell + sim->max_player_cells;
	while (cell->enabled && cell < cell_end)
	{
		if (cell->mass_target >= 22)
		{
			Mass* shot = GetNextFreeShot(sim);
			if (shot == nullptr) break;
			shot->color_id = cell->color_id;
			shot->mass = 10;
			MassChange(cell, -12);
			Vector2 delta = mouse_pos - cell->pos;
			if (length2(delta) == 0) delta = {1, 0}; else delta = normalize(delta);
			float radius = EntityRadius(cell);
			shot->pos = cell->pos + delta * radius;
			shot->vel = cell->vel * 0.5f + delta * shot_speed;
			tdArrayAdd(sim->shots_changed, (uint16)(shot - sim->shots.ptr));
		}
		++cell;
	}
}

void PopCell(Sim* sim, Entity* cell)
{
	assert(sim);
	assert(cell);
	assert(cell->enabled);
	Player* player = PlayerFromCell(sim, cell);
	int32 new_cells = sim->max_player_cells - PlayerCellCount(sim, player);
	if (new_cells == 0) return;

	float total_mass = PlayerTotalMass(sim, player);
	float mass = cell->mass_target * 0.5f;
	MassChange(cell, -mass);

	Entity* cell_start = FirstPlayerCellDisabled(sim, player, cell);
	assert(cell_start);
	Entity* cell_end = cell_start + sim->max_player_cells;

	Entity* orig_cell = cell;
	cell = cell_start;
	mass *= 0.5f;
	while (cell < cell_end)
	{
		memclear<Entity>(cell);
		cell->state = 2;
		cell->pos = orig_cell->pos;
		cell->pos.x += (float)(tdRandomNextDouble(sim->rng) - 0.5);
		cell->pos.y += (float)(tdRandomNextDouble(sim->rng) - 0.5);
		//assert(mass >= 8);
		MassSet(cell, mass);
		cell->merge_time = sim->total_seconds + MergeDelayPrimary(sim, cell->mass_target);
		if (mass < 16) break; else mass *= 0.5f;
		++cell;
	}
	sim->next_socket_write_time	= 0;
}

void UpdateAgarCellCollision(Sim* sim)
{
	TIMED_BLOCK(UpdateAgarCellCollision);
	float radius;
	Agar* agar;
	Entity* cell, *cell_end;
	Player* player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			agar = sim->agar.ptr;
			for (int32 j = 0; j < sim->agar.count; ++j, ++agar)
			{
				if (agar->enabled)
				{
					if (agar->x >= player->extent.x && agar->x <= player->extent.z &&
						agar->y >= player->extent.y && agar->y <= player->extent.w)
					{
						cell = FirstPlayerCell(sim, player);
						cell_end = cell + sim->max_player_cells;
						//Vector2 agar_pos = { agar->x, agar->y };
						while (cell < cell_end)
						{
							if (cell->enabled)
							{
								radius = EntityRadius(cell);
								if (distance(Vector2(agar->x, agar->y), cell->pos) <= radius)
								{
									tdArrayAdd(sim->agar_changed_thread_buffer, j);
									agar->state = 0;
									cell->mass += 1;
									cell->mass_target += 1;
									break;
								}
							}
							else
								break;
							++cell;
						}
					}
				}
			}
		}
		++player;
	}
}

DWORD WINAPI Thread1Execute(LPVOID lp)
{
	Sim *sim = (Sim*)lp;
	++sim->threads_running;
	while (sim->is_active)
	{
		if (!sim->is_paused)
			UpdateAgarCellCollision(sim);
		else
			Sleep(1);
	}
	--sim->threads_running;
	return 0;
}

DWORD WINAPI Thread2Execute(LPVOID lp)
{
	Sim *sim = (Sim*)lp;
	++sim->threads_running;
	while (sim->is_active)
	{
		if (!sim->is_paused)
			UpdateBots(sim);
		else
			Sleep(1);
	}
	--sim->threads_running;
	return 0;
}

void UpdateSim(Sim* sim)
{
	auto app_state = GetAppState();
	float elapsed = sim->seconds;
	Agar* agar;
	Mass* shot, *virus;
	Entity* cell, *spawner;
	Player* player;
	float len, radius, speed, dist;
	Vector2 delta;
	int32 half_world_size = sim->world_size / 2;
	float eat_fac = 1.25f;
	float eat_rad_fac = 0.25f;
	double separation_speed = 10.0; // lower = slower
	float vfac = (1.f - 2.f * elapsed);

	{
	TIMED_BLOCK(UpdatePlayerKeys);
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i, ++player)
	{
		if (player->state_data.keys.split)
		{
			PlayerSplit(sim, player);
			continue;
		}			
		if (player->state_data.keys.shoot)
		{
			if (sim->total_seconds >= player->next_shot_time)
			{
				PlayerShoot(sim, player);
				continue;
			}
		}
		if (player->gamer)
		{
			if (player->state_data.keys.pause_movement)
			{
				player->gamer->movement_paused = !player->gamer->movement_paused;
				continue;
			}			
			if (player->state_data.keys.debug_merge)
			{
				cell = FirstPlayerCell(sim, player);
				Entity* cell_end = cell + sim->max_player_cells;
				while (cell->enabled && cell < cell_end)
				{
					cell->merge_time = 0;
					++cell;
				}
				continue;
			}			
			if (player->state_data.keys.debug_dbl_mass)
			{
				cell = FirstPlayerCell(sim, player);
				MassChange(cell, cell->mass_target * 2);
				continue;
			}			
			if (player->state_data.keys.debug_add_mass_1k)
			{
				cell = FirstPlayerCell(sim, player);
				MassChange(cell, 1000);
				continue;
			}			
			if (player->state_data.keys.debug_rem_mass_1k)
			{
				cell = FirstPlayerCell(sim, player);
				if (cell->mass_target > 1010)
				{
					MassChange(cell, -1000);
				}
				continue;
			}			
			if (player->state_data.keys.debug_goto_pos)
			{
				cell = FirstPlayerCell(sim, player);
				Entity* cell_end = cell + sim->max_player_cells;
				Vector2 diff = Vector2(player->state_data.mouse_pos_x, player->state_data.mouse_pos_y) - player->center;
				while (cell < cell_end)
				{
					if (cell->enabled)
					{
						cell->state = 2;
						cell->pos += diff;
						++cell;
					}
					else
						break;
				}
				continue;
			}			
			if (player->state_data.keys.debug_slow_time)
			{
				if (sim->sim_speed < 1)
					sim->sim_speed = 1;
				else
					sim->sim_speed = 0.1f;
				continue;
			}			
		}
	}
	}

	{
	TIMED_BLOCK(UpdateCellMassChangeLerp);
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			while (cell < cell_end)
			{
				if (cell->enabled)
				{
					if (cell->mass_change_time_orig)
					{
						double target_time = cell->mass_change_time_orig + g_mass_lerp_time;;
						if (sim->total_seconds < target_time)
						{
							float a = (sim->total_seconds - cell->mass_change_time_orig) / (target_time - cell->mass_change_time_orig);
							cell->mass = tdLerp(cell->mass_change_orig, cell->mass_target, a);
						}
						else
						{
							cell->mass_target = cell->mass = (int)cell->mass_target;
							cell->mass_change_time_orig = 0;
							if (cell->mass >= sim->max_mass_per_cell)
							{
								if (!PlayerSplitCell(sim, player, cell, RandomWorldPosition(sim)))
									cell->mass = cell->mass_target = sim->max_mass_per_cell;
							}
						}
					}
				}
				else
					break;
				++cell;
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdateSpawnerMassChangeLerp);
	spawner = sim->spawners.ptr;
	Entity* spawner_end = spawner + sim->spawners.count;
	while (spawner < spawner_end)
	{
		if (spawner->enabled)
		{
			if (spawner->mass_change_time_orig)
			{
				double target_time = spawner->mass_change_time_orig + g_mass_lerp_time;;
				if (sim->total_seconds < target_time)
				{
					float a = (sim->total_seconds - spawner->mass_change_time_orig) / (target_time - spawner->mass_change_time_orig);
					spawner->mass = tdLerp(spawner->mass_change_orig, spawner->mass_target, a);
				}
				else
				{
					spawner->mass_target = spawner->mass = (int)spawner->mass_target;
					spawner->mass_change_time_orig = 0;
				}
			}
		}
		++spawner;
	}
	}

	{
	TIMED_BLOCK(UpdateAgarMoving)
	float vfac2 = (1.f - 2.f * elapsed);
	AgarMoving* moving = sim->agar_moving.ptr;
	for (int i = 0; i < sim->agar_moving.count; ++i, ++moving)
	{
		if (moving->enabled)
		{
			moving->pos.x += moving->vel.x * elapsed;
			moving->pos.y += moving->vel.y * elapsed;
			moving->vel.x *= vfac2;
			moving->vel.y *= vfac2;
			if (length2(moving->vel) < 0.1f)
			{
				agar = sim->agar.ptr + moving->agar_id;
				agar->x = (int16)moving->pos.x;
				agar->y = (int16)moving->pos.y;
				agar->color_id = moving->color_id;
				moving->state = 0;
			}
		}
	}
	}

	{
	TIMED_BLOCK(UpdateViruses)
	float t, r2 = EntityRadius(g_virus_mass) * eat_rad_fac;
	float vfac2 = (1.f - 2.f * elapsed);
	virus = sim->viruses.ptr;
	for (int i = 0; i < sim->viruses.count; ++i, ++virus)
	{
		if (virus->enabled && (virus->vel.x || virus->vel.y))
		{
			virus->pos.x += virus->vel.x * elapsed;
			virus->pos.y += virus->vel.y * elapsed;
			virus->vel.x *= vfac2;
			virus->vel.y *= vfac2;
			if (length2(virus->vel) < 0.1f) virus->vel = {};

			spawner = sim->spawners.ptr;
			for (int j = 0; j < sim->spawners.count; ++j, ++spawner)
			{
				if (spawner->enabled)
				{
					radius = EntityRadius(spawner);
					dist = distance2(spawner->pos, virus->pos);
					t = radius - r2;
					if (dist < t * t)
					{
						MassChange(spawner, virus->mass);
						if (spawner->state == 1)
						{
							tdArrayAdd(sim->spawners_changed, (uint16)j);
							if (!tdArrayContains(sim->spawners_emitting, (uint16)j)) tdArrayAdd(sim->spawners_emitting, (uint16)j);
							spawner->state = 2;
						}

						uint st = virus->state;
						memclear<Mass>(virus);
						virus->pos = RandomWorldPositionAwayFromEntities(sim);
						virus->state = 1;
						virus->mass = g_virus_mass;
						if (st == 1)
						{
							tdArrayAdd(sim->viruses_changed, (uint16)i);
							virus->state = 2;
						}
						break;
					}
				}
			}
		}
	}
	}

	{
	TIMED_BLOCK(UpdateShots)
	shot = sim->shots.ptr;
	for (int i = 0; i < sim->shots.count; ++i, ++shot)
	{
		if (shot->enabled && (shot->vel.x || shot->vel.y))
		{
			shot->pos.x += shot->vel.x * elapsed;
			shot->pos.y += shot->vel.y * elapsed;
			shot->vel.x *= vfac;
			shot->vel.y *= vfac;
			if (length2(shot->vel) < 0.1f) shot->vel = {};
		}
	}
	}

	{
	TIMED_BLOCK(UpdateCellPositions);
	Vector2 mouse_pos;
	float vfac2 = (1.f - 5.f * elapsed);
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			float debug_speed_boost = player->state_data.keys.debug_speed_boost ? 5 : 1;
			mouse_pos = MousePos(player);
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			while (cell < cell_end && cell->enabled)
			{
				if (cell->vel.x || cell->vel.y)
				{
					cell->pos.x += cell->vel.x * elapsed;
					cell->pos.y += cell->vel.y * elapsed;
					cell->vel.x *= vfac2;
					cell->vel.y *= vfac2;
					if (length2(cell->vel) < 1.f) cell->vel = {};
				}

				radius = EntityRadius(cell);
				delta = mouse_pos - cell->pos;
				len = length(delta);
				if (len > 0.f)
				{
					delta = normalize(delta);
					if (len > radius) len = radius;
					speed = pow(cell->mass, -0.20) * 100;
					cell->pos += delta * ((len / radius) * speed * elapsed * debug_speed_boost);
				}

				float diff = length(cell->pos) + radius * 0.5 - half_world_size;
				if (diff > 0)
				{
					cell->pos -= normalize(cell->pos) * diff;
				}
				++cell;
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdateCellSeparationAndMerge);
	float r2, t;
	Entity* cell_end, *cell_endx, *cell2;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			cell_endx = cell_end - 1;
			while (cell < cell_endx && cell->enabled)
			{
				cell2 = cell + 1;
				radius = EntityRadius(cell);
				bool sep = cell->merge_time > sim->total_seconds;
				while (cell2 < cell_end && cell2->enabled)
				{
					r2 = EntityRadius(cell2);
					if (sep || (cell2->merge_time > sim->total_seconds))
					{
						if (!IsCellMoving(cell) && !IsCellMoving(cell2))
						{
							// separate
							delta = cell2->pos - cell->pos;
							len = length(delta);
							if (len < radius + r2)
							{
								delta = normalize(delta);
								double hdr = (radius + r2 - len);// * 0.5;
								double adr = (radius + r2) * separation_speed * elapsed;
								if (adr > hdr) adr = hdr;
								double dx = delta.x * adr;
								double dy = delta.y * adr;
								Vector2 ratio = normalize(Vector2(cell->mass / cell2->mass, cell2->mass / cell->mass));//{0.5f, 0.5f};//
								cell->pos.x -= dx * ratio.y;
								cell->pos.y -= dy * ratio.y;
								cell2->pos.x += dx * ratio.x;
								cell2->pos.y += dy * ratio.x;
							}
						}
					}
					else
					{
						// merge
						dist = distance2(cell->pos, cell2->pos);
						if (cell->mass_target >= cell2->mass_target)
						{
							t = radius - r2 * eat_rad_fac;
							if (dist < t * t)
							{
								MassChange(cell, cell2->mass_target);
								cell->merge_time += MergeDelaySecondary(sim, cell2->mass_target) * 0.5f;
								DisableCell(sim, cell2);
								goto next_player;
							}
						}
						else
						{
							t = r2 - radius * eat_rad_fac;
							if (dist < t * t)
							{
								MassChange(cell2, cell->mass_target);
								cell2->merge_time += MergeDelaySecondary(sim, cell->mass_target) * 0.5f;
								DisableCell(sim, cell);
								goto next_player;
							}
						}
					}
					++cell2;
				}
				++cell;
			}
		}
		next_player:
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdatePlayerExtents);
	Vector2 min, max;
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			min.x = min.y = FLT_MAX;
			max.x = max.y = -FLT_MAX;
			while (cell < cell_end)
			{
				if (cell->enabled)
				{
					radius = EntityRadius(cell);
					assert(!isnan(radius));
					if (cell->pos.x - radius < min.x) min.x = cell->pos.x - radius;
					if (cell->pos.x + radius > max.x) max.x = cell->pos.x + radius;
					if (cell->pos.y - radius < min.y) min.y = cell->pos.y - radius;
					if (cell->pos.y + radius > max.y) max.y = cell->pos.y + radius;
				}
				else
					break;
				++cell;
			}
			player->extent.x = min.x;
			player->extent.y = min.y;
			player->extent.z = max.x;
			player->extent.w = max.y;
			player->center.x = min.x + (max.x - min.x) * 0.5f;
			player->center.y = min.y + (max.y - min.y) * 0.5f;
			assert(!isinf(player->center.x));
			assert(!isinf(player->center.y));
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdateShotCollision);
	radius = EntityRadius(g_shot_mass);
	float r2, rv = EntityRadius(g_virus_mass);
	Entity* cell_end;
	shot = sim->shots.ptr;
	for (int32 i = 0; i < sim->shots.count; ++i)
	{
		if (shot->enabled)
		{
			virus = sim->viruses.ptr;
			for (int j = 0; j < sim->viruses.count; ++j, ++virus)
			{
				if (virus->enabled)
				{
					if (distance(shot->pos, virus->pos) <= rv)
					{
						virus->vel = shot->vel * 0.25f;
						if (virus->state == 1)
						{
							tdArrayAdd(sim->viruses_changed, (uint16)j);
							virus->state = 2;
						}

						shot->enabled = false;
						tdArrayAdd(sim->shots_changed, (uint16)i);
						goto next_shot;
					}
				}
			}

			spawner = sim->spawners.ptr;
			for (int j = 0; j < sim->spawners.count; ++j, ++spawner)
			{
				if (spawner->enabled)
				{
					r2 = EntityRadius(spawner);
					if (distance(shot->pos, spawner->pos) <= r2)
					{
						MassChange(spawner, shot->mass);
						if (spawner->state == 1)
						{
							tdArrayAdd(sim->spawners_changed, (uint16)j);
							if (!tdArrayContains(sim->spawners_emitting, (uint16)j)) tdArrayAdd(sim->spawners_emitting, (uint16)j);
							spawner->state = 2;
						}

						shot->enabled = false;
						tdArrayAdd(sim->shots_changed, (uint16)i);
						goto next_shot;
					}
				}
			}

			player = sim->players.ptr;
			for (int j = 0; j < sim->players.count; ++j, ++player)
			{
				if (player->type)
				{
					if (shot->pos.x + radius >= player->extent.x && shot->pos.x - radius <= player->extent.z &&
						shot->pos.y + radius >= player->extent.y && shot->pos.y - radius <= player->extent.w)
					{
						cell = FirstPlayerCell(sim, player);
						cell_end = cell + sim->max_player_cells;
						while (cell < cell_end)
						{
							if (cell->enabled)
							{
								r2 = EntityRadius(cell);
								if (distance(shot->pos, cell->pos) <= r2)
								{
									MassChange(cell, shot->mass);
									shot->enabled = false;
									tdArrayAdd(sim->shots_changed, (uint16)i);
									goto next_shot;
								}
							}
							else
								break;
							++cell;
						}
					}
				}
			}
		}
		next_shot:
		++shot;
	}
	}

	{
	TIMED_BLOCK(UpdateVirusCellCollision);
	float r2, t, d2;
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			virus = sim->viruses.ptr;
			for (int32 j = 0; j < sim->viruses.count; ++j, ++virus)
			{
				if (virus->enabled)
				{
					r2 = EntityRadius(virus);
					if (virus->pos.x + r2 >= player->extent.x && virus->pos.x - r2 <= player->extent.z &&
						virus->pos.y + r2 >= player->extent.y && virus->pos.y - r2 <= player->extent.w)
					{
						cell = FirstPlayerCell(sim, player);
						cell_end = cell + sim->max_player_cells;
						while (cell < cell_end)
						{
							if (cell->enabled)
							{
								radius = EntityRadius(cell);
								if (cell->mass_target >= virus->mass * eat_fac)
								{
									d2 = distance2(cell->pos, virus->pos);
									t = radius - r2 * eat_rad_fac;
									if (d2 < t * t)
									{
										MassChange(cell, virus->mass);
										uint8 st = virus->state;
										memclear<Mass>(virus);
										PopCell(sim, cell);
										virus->pos = RandomWorldPositionAwayFromEntities(sim);
										virus->state = 1;
										virus->mass = g_virus_mass;
										if (st == 1)
										{
											tdArrayAdd(sim->viruses_changed, (uint16)j);
											virus->state = 2;
										}
										break;
									}
								}
							}
							else
								break;
							++cell;
						}
					}
				}
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdateSpawnerCellCollision);
	float r2, t, d2;
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			spawner = sim->spawners.ptr;
			for (int32 j = 0; j < sim->spawners.count; ++j, ++spawner)
			{
				if (spawner->enabled)
				{
					r2 = EntityRadius(spawner);
					if (spawner->pos.x + r2 >= player->extent.x && spawner->pos.x - r2 <= player->extent.z &&
						spawner->pos.y + r2 >= player->extent.y && spawner->pos.y - r2 <= player->extent.w)
					{
						cell = FirstPlayerCell(sim, player);
						cell_end = cell + sim->max_player_cells;
						while (cell < cell_end)
						{
							if (cell->enabled)
							{
								radius = EntityRadius(cell);
								if (cell->mass_target >= spawner->mass * eat_fac)
								{
									d2 = distance2(cell->pos, spawner->pos);
									t = radius - r2 * eat_rad_fac;
									if (d2 < t * t)
									{
										MassChange(cell, spawner->mass);
										uint8 st = spawner->state;
										memclear<Entity>(spawner);
										PopCell(sim, cell);
										spawner->pos = RandomWorldPositionAwayFromEntities(sim);
										spawner->state = 1;
										spawner->mass = spawner->mass_target = g_spawner_mass;
										if (st == 1)
										{
											tdArrayAdd(sim->spawners_changed, (uint16)j);
											spawner->state = 2;
										}
										break;
									}
								}
								else if (spawner->mass >= cell->mass_target * eat_fac)
								{
									d2 = distance2(cell->pos, spawner->pos);
									t = r2 - radius * eat_rad_fac;
									if (d2 < t * t)
									{
										float mass_change = min(spawner->mass_target + cell->mass_target, sim->max_mass_per_cell * 0.5f) - spawner->mass_target;
										if (mass_change)
										{
											MassChange(spawner, mass_change);
											if (spawner->state == 1)
											{
												tdArrayAdd(sim->spawners_changed, (uint16)j);
												if (!tdArrayContains(sim->spawners_emitting, (uint16)j)) tdArrayAdd(sim->spawners_emitting, (uint16)j);
												spawner->state = 2;
											}
										}
										DisableCell(sim, cell);
										break;
									}
								}
							}
							else
								break;
							++cell;
						}
					}
				}
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdatePlayerCollision);
	Entity* cell_start1, *cell_end1, *cell_start2, *cell_end2, *cell2;
	Vector4 extent1;
	float r2, t, d2;
	player = sim->players.ptr;
	Player* player2;
	for (int i = 0; i < sim->players.count - 1; ++i)
	{
		if (player->type)
		{
			cell_start1 = FirstPlayerCell(sim, player);
			cell_end1 = cell_start1 + sim->max_player_cells;

			player2 = sim->players.ptr + i + 1;
			for (int j = i + 1; j < sim->players.count; ++j)
			{
				if (player2->type && player2 != player)
				{
					if (tdIntersectExtent(player->extent, player2->extent))
					{
						cell_start2 = FirstPlayerCell(sim, player2);
						cell_end2 = cell_start2 + sim->max_player_cells;

						cell = cell_start1;
						while (cell < cell_end1 && cell->enabled)
						{
							radius = EntityRadius(cell);
							extent1.x = cell->pos.x - radius;
							extent1.y = cell->pos.y - radius;
							extent1.z = cell->pos.x + radius;
							extent1.w = cell->pos.y + radius;
							if (tdIntersectExtent(extent1, player2->extent))
							{
								cell2 = cell_start2;
								while (cell2 < cell_end2 && cell2->enabled)
								{
									if (cell->mass_target >= cell2->mass_target * eat_fac)
									{
										d2 = distance2(cell->pos, cell2->pos);
										r2 = EntityRadius(cell2);
										t = radius - r2 * eat_rad_fac;
										if (d2 < t * t)
										{
											MassChange(cell, cell2->mass_target);
											DisableCell(sim, cell2);
											cell = cell_end1;
											break;
										}
									}
									else if (cell2->mass_target >= cell->mass_target * eat_fac)
									{
										d2 = distance2(cell->pos, cell2->pos);
										r2 = EntityRadius(cell2);
										t = r2 - radius * eat_rad_fac;
										if (d2 < t * t)
										{
											MassChange(cell2, cell->mass_target);
											DisableCell(sim, cell);
											cell = cell_end1;
											break;
										}
									}
									++cell2;
								}
							}
							++cell;
						}
					}
				}
				++player2;
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdatePlayerTotals);
	Entity* cell_end;
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{		
		if (player->type)
		{
			player->total_mass = 0;
			player->cell_count = 0;
			cell = FirstPlayerCell(sim, player);
			cell_end = cell + sim->max_player_cells;
			while (cell < cell_end)
			{
				if (cell->enabled)
				{
					player->total_mass += cell->mass_target;
					++player->cell_count;
					++cell;
				}
				else
					break;
			}
		}
		++player;
	}
	}

	{
	TIMED_BLOCK(UpdateAgarRespawn);
	///TODO not thread safe	
	tdArrayCopy(sim->agar_changed, sim->agar_changed_thread_buffer);
	tdArrayClear(sim->agar_changed_thread_buffer);	
	for (int i = 0; i < sim->agar_changed.count; ++i)
	{
		agar = sim->agar.ptr + sim->agar_changed[i];
		if (sim->agar_changed_socket_buffer.count < sim->agar_changed_socket_buffer.cap) 
			tdArrayAdd(sim->agar_changed_socket_buffer, sim->agar_changed[i]);

		spawner = tdRandomNext(sim->rng, 2) ? GetRandomEmittingSpawner(sim) : nullptr;
		if (spawner)
		{
			AddMovingAgar(sim, spawner, agar);
			//agar->state = 0;
			--spawner->mass;
			--spawner->mass_target;
			if (spawner->state == 1)
			{
				tdArrayAdd(sim->spawners_changed, (uint16)(spawner - sim->spawners.ptr));
				spawner->state = 2;
			}
		}
		else
		{
			agar->color_id = (uint8)tdRandomNext(sim->rng, agar_color_count - 4) + 4;
			retry:
			agar->x = (int16)tdRandomNext(sim->rng, sim->world_size) - half_world_size;
			agar->y = (int16)tdRandomNext(sim->rng, sim->world_size) - half_world_size;
			if (length(Vector2(agar->x, agar->y)) >= half_world_size) goto retry;
		}
	}
	sim->agar_changed.count = 0;
	}

	{
	TIMED_BLOCK(ClearPlayerStateData);
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (!IsBot(player))
		{
			Keys& k = sim->players[i].state_data.keys;
			k.split = false;
			k.debug_merge = false;
			k.debug_dbl_mass = false;
			k.debug_add_mass_1k = false;
			k.debug_rem_mass_1k = false;
			k.debug_goto_pos = false;
			k.debug_slow_time = false;
		}
	}
	}

	{
	TIMED_BLOCK(UpdateLeaderboard);
	sim->leader_board_changed = false;
    tdArrayClear(sim->leader_board);
	player = sim->players.ptr;
	for (int i = 0; i < sim->players.count; ++i)
	{
		if (player->type)
		{
			for (int32 j = 0; j < sim->leader_board.count; ++j)
			{
				if (player->total_mass > sim->players[sim->leader_board[j] - 1].total_mass)
				{
					if (sim->leader_board.count == sim->leader_board.cap) --sim->leader_board.count;
					tdArrayInsert(sim->leader_board, (int16)(i + 1), j);
					goto next_player1;
				}
			}
			if (sim->leader_board.count < sim->leader_board.cap)
				tdArrayAdd(sim->leader_board, (int16)(i + 1));
		}
		next_player1:
		++player;
	}
	if (sim->leader_board.count != sim->leader_board_prev.count)
		sim->leader_board_changed = true;
	else
	{
		for (int i = 0; i < sim->leader_board.count; ++i)
		{
			if (sim->leader_board[i] != sim->leader_board_prev[i])
			{
				sim->leader_board_changed = true;
				break;
			}
		}
	}
	}
}

void SimNew()
{
	auto app_state = GetAppState();
	Sim* sim = &app_state->sim;
	memclear(sim);
	sim->is_active = true;
	sim->is_paused = false;
	sim->sim_speed = 1;//0.05;
	sim->max_world_size = 2048;
	sim->max_player_cells = 16;
	sim->max_mass_per_cell = 22500;

	sim->world_size = sim->max_world_size;

	uint64 seed = ((uint64)tdRandomNext(app_state->rng) << 32) + tdRandomNext(app_state->rng);
	uint64 inc = ((uint64)tdRandomNext(app_state->rng) << 32) + tdRandomNext(app_state->rng);
	tdRandomSeed(&sim->rng, seed, inc);

	tdArrayInit(sim->gamers, app_state->main_arena, 5000);
	tdArrayInit(sim->players, app_state->main_arena, sim->gamers.cap);
	tdArrayInit(sim->player_cells, app_state->main_arena, sim->gamers.cap * sim->max_player_cells);
	tdArrayInit(sim->agar, app_state->main_arena, 8000);
	tdArrayInit(sim->agar_changed, app_state->main_arena, sim->agar.cap / 2);
	tdArrayInit(sim->agar_changed_thread_buffer, app_state->main_arena, sim->agar_changed.cap);
	tdArrayInit(sim->agar_changed_socket_buffer, app_state->main_arena, sim->agar_changed.cap);
	tdArrayInit(sim->agar_moving, app_state->main_arena, sim->agar.cap / 2);
	tdArrayInit(sim->shots, app_state->main_arena, sim->player_cells.cap);
	tdArrayInit(sim->shots_changed, app_state->main_arena, sim->shots.cap / 2);
	tdArrayInit(sim->viruses, app_state->main_arena, 140);//2200);
	tdArrayInit(sim->viruses_changed, app_state->main_arena, sim->viruses.cap);
	tdArrayInit(sim->spawners, app_state->main_arena, 30);//500);
	tdArrayInit(sim->spawners_changed, app_state->main_arena, sim->spawners.cap);
	tdArrayInit(sim->spawners_emitting, app_state->main_arena, sim->spawners.cap);
	tdArrayInit(sim->connections, app_state->main_arena, sim->gamers.cap);
	tdArrayInit(sim->leader_board, app_state->main_arena, 10);
	tdArrayInit(sim->leader_board_prev, app_state->main_arena, 10);
	tdArrayInit(sim->bot_data, app_state->main_arena, sim->players.cap);

	sim->player_name_text = tdMalloc<char>(app_state->main_arena, sim->players.cap * 21);
	sim->player_mass_text = tdMalloc<char>(app_state->main_arena, sim->player_cells.cap * 6);

	Agar* agar = sim->agar.ptr;
	sim->agar.count = sim->agar.cap;
	for (int i = 0; i < sim->agar.count; ++i)
	{
		agar->color_id = (uint8)tdRandomNext(sim->rng, agar_color_count - 4) + 4;
		Vector2 pos = RandomWorldPosition(sim);
		agar->x = (int16)pos.x;
		agar->y = (int16)pos.y;
		++agar;
	}
	
	Mass* virus = sim->viruses.ptr;
	sim->viruses.count = sim->viruses.cap;
	for (int i = 0; i < sim->viruses.count; ++i)
	{
		virus->enabled = true;
		virus->pos = RandomWorldPosition(sim);
		virus->mass = g_virus_mass;
		++virus;
	}	
	
	Entity* spawner = sim->spawners.ptr;
	sim->spawners.count = sim->spawners.cap;
	for (int i = 0; i < sim->spawners.count; ++i)
	{
		spawner->enabled = true;
		spawner->pos = RandomWorldPosition(sim);
		spawner->mass = spawner->mass_target = g_spawner_mass;
		++spawner;
	}

	HANDLE handle1 = CreateThread(0, 0, Thread1Execute, sim, 0, 0);
	HANDLE handle2 = CreateThread(0, 0, Thread2Execute, sim, 0, 0);
	CloseHandle(handle1);
	CloseHandle(handle2);
}

void SimEnd()
{
	auto app_state = GetAppState();
	app_state->sim.is_active = false;
	while (app_state->sim.threads_running > 0);
	if (app_state->socket) closesocket(app_state->socket);
}

