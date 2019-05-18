
void UpdateSim(Sim* sim)
{
}

void SimNew()
{
	auto app_state = GetAppState();
	Sim* sim = &app_state->sim;
	memclear(sim);
	sim->is_active = true;
	sim->is_paused = false;
	sim->sim_speed = 1;//0.05;

	uint64 seed = ((uint64)tdRandomNext(app_state->rng) << 32) + tdRandomNext(app_state->rng);
	uint64 inc = ((uint64)tdRandomNext(app_state->rng) << 32) + tdRandomNext(app_state->rng);
	tdRandomSeed(&sim->rng, seed, inc);

	tdArrayInit(sim->players, app_state->main_arena, 1);
	PlayerNew(sim, app_state->gamers.ptr);
}

void SimEnd()
{
	auto app_state = GetAppState();
	app_state->sim.is_active = false;
	//while (app_state->sim.threads_running > 0);
}

