
void WriteSimDef(const Sim* sim, uint8** stream)
{
    WriteBin<int32>(stream, sim->max_world_size);
    WriteBin<int32>(stream, sim->max_player_cells);
    WriteBin<int32>(stream, sim->max_mass_per_cell);
    WriteBin<int32>(stream, sim->players.cap);
    WriteBin<int32>(stream, sim->agar.cap);
    WriteBin<int32>(stream, sim->shots.cap);
    WriteBin<int32>(stream, sim->viruses.cap);
    WriteBin<int32>(stream, sim->spawners.cap);
    WriteBin<int32>(stream, 20);
    WriteBin<int32>(stream, sim->leader_board.cap);
}

void WriteFullSimState(const Sim* sim, uint8** stream)
{
    WriteBin<double>(stream, sim->total_seconds);
    WriteBin<int32>(stream, sim->world_size);

    Player* p = sim->players.ptr;
    char* name_text = sim->player_name_text;
    WriteBin<int32>(stream, sim->players.count);
	for (int32 i = 0; i < sim->players.count; ++i, ++p)
	{
        WriteBin<uint8>(stream, p->type);
        WriteBinArray<char>(stream, name_text, 20);
        name_text += 21;
	}

    Entity* cell = sim->player_cells.ptr;
    WriteBin<int32>(stream, sim->player_cells.count);
    for (int32 i = 0; i < sim->player_cells.count; ++i, ++cell)
    {
        WriteBin<uint8>(stream, cell->state);
        if (cell->enabled)
        {
            WriteBin<float>(stream, cell->pos.x);
            WriteBin<float>(stream, cell->pos.y);
            WriteBin<float>(stream, cell->mass_target);
        }
    }

    Agar* agar = sim->agar.ptr;
    for (int32 i = 0; i < sim->agar.cap; ++i, ++agar)
    {
        WriteBin<uint8>(stream, agar->state);
        if (agar->enabled)
        {
            WriteBin<int16>(stream, agar->x);
            WriteBin<int16>(stream, agar->y);
        }
    }

    Mass* shot = sim->shots.ptr;
    WriteBin<int32>(stream, sim->shots.count);
    for (int32 i = 0; i < sim->shots.count; ++i, ++shot)
    {
        WriteBin<uint8>(stream, shot->state);
        if (shot->enabled)
        {
            WriteBin<float>(stream, shot->pos.x);
            WriteBin<float>(stream, shot->pos.y);
            WriteBin<float>(stream, shot->vel.x);
            WriteBin<float>(stream, shot->vel.y);
        }
    }

    Mass* virus = sim->viruses.ptr;
    WriteBin<int32>(stream, sim->viruses.count);
    for (int32 i = 0; i < sim->viruses.count; ++i, ++virus)
    {
        WriteBin<uint8>(stream, virus->state);
        if (virus->enabled)
        {
            WriteBin<float>(stream, virus->pos.x);
            WriteBin<float>(stream, virus->pos.y);
            WriteBin<float>(stream, virus->vel.x);
            WriteBin<float>(stream, virus->vel.y);
        }
    }

    Entity* spawner = sim->spawners.ptr;
    WriteBin<int32>(stream, sim->spawners.count);
    for (int32 i = 0; i < sim->spawners.count; ++i, ++spawner)
    {
        WriteBin<uint8>(stream, spawner->state);
        if (spawner->enabled)
        {
            WriteBin<float>(stream, spawner->pos.x);
            WriteBin<float>(stream, spawner->pos.y);
            WriteBin<uint16>(stream, (uint16)spawner->mass_target);
        }
    }
}

void WriteCurrentSimState(Sim* sim, uint8** stream)
{
    WriteBin<double>(stream, sim->total_seconds);
    WriteBin<int32>(stream, sim->world_size);

    WriteBin<int32>(stream, sim->players.count);
    Player* player = sim->players.ptr;
    for (int32 i = 0; i < sim->players.count; ++i, ++player)
    {
        WriteBin<uint8>(stream, player->type);
        if (player->type)
        {
            WriteBin<int16>(stream, player->state_data.mouse_pos_x);
            WriteBin<int16>(stream, player->state_data.mouse_pos_y);
            if (player->type == 2)
            {
                char* name_text = sim->player_name_text + i * 21;
                WriteBinArray<char>(stream, name_text, 20);
                player->type = 1;
            }
        }
    }

    WriteBin<int32>(stream, sim->player_cells.count);
    Entity* cell = sim->player_cells.ptr;
    for (int32 i = 0; i < sim->player_cells.count; ++i, ++cell)
    {
        WriteBin<uint8>(stream, cell->state);
        if (cell->enabled)
        {
            WriteBin<float>(stream, cell->pos.x);
            WriteBin<float>(stream, cell->pos.y);
            WriteBin<float>(stream, cell->mass_target);
            cell->state = 1;
        }
    }

    WriteBin<int32>(stream, sim->agar_changed_socket_buffer.count);
    Agar* agar;
    for (int32 i = 0; i < sim->agar_changed_socket_buffer.count; ++i)
    {
        agar = sim->agar.ptr + sim->agar_changed_socket_buffer[i];
        WriteBin<int32>(stream, sim->agar_changed_socket_buffer[i]);
        WriteBin<uint8>(stream, agar->state);
        if (agar->enabled)
        {
            WriteBin<int16>(stream, agar->x);
            WriteBin<int16>(stream, agar->y);
        }
    }

    WriteBin<int32>(stream, sim->shots.count);
    WriteBin<int32>(stream, sim->shots_changed.count);
    Mass* shot;
    for (int32 i = 0; i < sim->shots_changed.count; ++i)
    {
        WriteBin<uint16>(stream, sim->shots_changed[i]);
        shot = sim->shots.ptr + sim->shots_changed[i];
        WriteBin<uint8>(stream, shot->state);
        if (shot->enabled)
        {
            WriteBin<float>(stream, shot->pos.x);
            WriteBin<float>(stream, shot->pos.y);
            WriteBin<float>(stream, shot->vel.x);
            WriteBin<float>(stream, shot->vel.y);
        }
    }

    WriteBin<int32>(stream, sim->viruses.count);
    WriteBin<int32>(stream, sim->viruses_changed.count);
    Mass* virus;
    for (int32 i = 0; i < sim->viruses_changed.count; ++i)
    {
        WriteBin<uint16>(stream, sim->viruses_changed[i]);
        virus = sim->viruses.ptr + sim->viruses_changed[i];
        WriteBin<uint8>(stream, virus->state);
        if (virus->enabled)
        {
            WriteBin<float>(stream, virus->pos.x);
            WriteBin<float>(stream, virus->pos.y);
            WriteBin<float>(stream, virus->vel.x);
            WriteBin<float>(stream, virus->vel.y);
            virus->state = 1;
        }
    }

    WriteBin<int32>(stream, sim->spawners.count);
    WriteBin<int32>(stream, sim->spawners_changed.count);
    Entity* spawner;
    for (int32 i = 0; i < sim->spawners_changed.count; ++i)
    {
        WriteBin<uint16>(stream, sim->spawners_changed[i]);
        spawner = sim->spawners.ptr + sim->spawners_changed[i];
        WriteBin<uint8>(stream, spawner->state);
        if (spawner->enabled)
        {
            WriteBin<float>(stream, spawner->pos.x);
            WriteBin<float>(stream, spawner->pos.y);
            WriteBin<uint16>(stream, (uint16)spawner->mass_target);
            spawner->state = 1;
        }
    }

    if (sim->leader_board_changed)
    {
        WriteBin<uint8>(stream, sim->leader_board.count);
        for (int32 i = 0; i < sim->leader_board.count; ++i)
            WriteBin<int16>(stream, sim->leader_board[i]);
    }
    else
        WriteBin<uint8>(stream, 0);
}

int CreateServerSocket()
{
    int result;
    auto app_state = GetAppState();

    if (!app_state->winsock_initialized)
    {
        result = WSAStartup(MAKEWORD(2, 2), &app_state->wsa);
        if (result != 0) return result;
        app_state->winsock_initialized = true;
    }

    app_state->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(app_state->socket == INVALID_SOCKET) return WSAGetLastError();

   unsigned long non_blocking = 1;
   result = ioctlsocket(app_state->socket, FIONBIO, &non_blocking);
   if (result != 0) return result;

   app_state->socket_addr = {};
   app_state->socket_addr.sin_family = AF_INET;
   app_state->socket_addr.sin_addr.s_addr = inet_addr(app_state->ip_address);
   app_state->socket_addr.sin_port = htons(app_state->port);

   result = bind(app_state->socket, (sockaddr*)&app_state->socket_addr, sizeof(app_state->socket_addr));
   if (result == SOCKET_ERROR)
   {
       closesocket(app_state->socket);
       return WSAGetLastError();
   }

   return 0;
}

Connection* GetConnection(Sim* sim, sockaddr_in* addr)
{
    assert(sim);
    Connection *c = sim->connections.ptr;
    for (int i = 0; i < sim->connections.count; ++i, c++)
    {
        if (memcmp(addr, &c->socket_addr, c->addr_len) == 0)
            return c;
    }
    return nullptr;
}

void CloseConnection(Sim* sim, sockaddr_in* addr)
{
    Connection *c = GetConnection(sim, addr);
    if (c)
    {
        if (c->gamer->player) 
        {
            c->gamer->player->type = 0;
            c->gamer->player = nullptr;
        }
        c->gamer->gamer_id = 0;
        tdArrayRemoveAt(sim->connections, c - sim->connections.ptr);
    }
}

void ReadSockets()
{
    auto app_state = GetAppState();
    assert(app_state->socket);
    Sim* sim = &app_state->sim;
    assert(sim->is_active);

    sockaddr_in client_addr = {};
    int client_addr_len = sizeof(client_addr);
    uint8* packet = app_state->packet;

retry:
    int recv_len = recvfrom(app_state->socket, (char*)packet, app_state->packet_size, 0, (sockaddr *)&client_addr, &client_addr_len);
    if (recv_len == SOCKET_ERROR)
    {
        int result = WSAGetLastError();
        if (result != WSAEWOULDBLOCK)
        {
            if (result == WSAECONNRESET)
            {
                CloseConnection(sim, &client_addr);
                goto retry;
            }
            sprintf(temp_text, "recvfrom() error: %d", WSAGetLastError());
            LogError(temp_text);
        }
        return;
    }
    app_state->bytes_recv_per_frame += recv_len;

    while (recv_len > 0)
    {
        int packet_length = ReadBin<int32>(&packet);
        recv_len -= packet_length;

        if (packet_length > sizeof(int32))
        {
            PacketType type = ReadBin<PacketType>(&packet);

            sprintf(temp_text, "packet received: %s, length: %d", packet_type_names[type], packet_length);
            LogError(temp_text);

            switch (type)
            {
                case pk_ClientState:
                    {
                        Connection* c = GetConnection(sim, &client_addr);
                        if (c)
                        {
                            assert(c->gamer);
                            assert(c->gamer->player);
                            c->gamer->player->state_data.mouse_pos_x = ReadBin<int16>(&packet);
                            c->gamer->player->state_data.mouse_pos_y = ReadBin<int16>(&packet);
                            c->gamer->player->state_data.keys = ReadBin<Keys>(&packet);
                        }
                    } break;
                case pk_ConnectRequest:
                    if (sim->connections.count < sim->connections.cap)
                    {
                        Gamer* gamer = GamerNew(sim);
                        if (gamer)
                        {
                            Connection* connection = tdArrayPush(sim->connections);
                            connection->gamer = gamer;
                            connection->socket = app_state->socket;
                            connection->addr_len = client_addr_len;
                            memcpy(&connection->socket_addr, &client_addr, sizeof(client_addr));

                            packet = app_state->packet;
                            WriteBin<int32>(&packet, 0);
                            WriteBin<PacketType>(&packet, pk_ConnectConfirm);
                            WriteBin<int16>(&packet, gamer->gamer_id);                            
                            WriteSimDef(sim, &packet);
                            WriteFullSimState(sim, &packet);

                            packet_length = packet - app_state->packet;
                            packet = app_state->packet;
                            WriteBin<int32>(&packet, packet_length);
                            packet = app_state->packet;

                            int result = sendto(connection->socket, (const char *)packet, packet_length, 0, (sockaddr *)&connection->socket_addr, connection->addr_len);
                            if (result == SOCKET_ERROR)
                            {
                                sprintf(temp_text, "sendto() error: ConnectConfirm: %d", WSAGetLastError());
                                LogError(temp_text);
                            }
                            else
                                app_state->bytes_sent_per_frame += result;
                        }
                    } break;
                case pk_PlayRequest:
                    {
                        int16 gamer_id = ReadBin<int16>(&packet);
                        if (gamer_id > 0 && gamer_id <= sim->gamers.count)
                        {
                            Gamer* gamer = sim->gamers.ptr + gamer_id - 1;
                            Player *player = PlayerNew(sim, gamer);

                            packet = app_state->packet;
                            WriteBin<int32>(&packet, 0);
                            WriteBin<PacketType>(&packet, pk_PlayConfirm);
                            int16 player_id = player - sim->players.ptr + 1;
                            WriteBin<int16>(&packet, player_id);
                            packet_length = packet - app_state->packet;
                            packet = app_state->packet;
                            WriteBin<int32>(&packet, packet_length);
                            packet = app_state->packet;

                            Connection *connection = GetConnection(sim, &client_addr);
                            if (connection)
                            {
                                connection->gamer = gamer;
                                int result = sendto(connection->socket, (const char*)packet, packet_length, 0, (sockaddr *)&connection->socket_addr, connection->addr_len);
                                if (result == SOCKET_ERROR)
                                {
                                    sprintf(temp_text, "sendto() error: PlayConfirm: %d", WSAGetLastError());
                                    LogError(temp_text);
                                }
                                else
                                    app_state->bytes_sent_per_frame += result;
                            }
                            else
                            {
                                LogError("could not find connection for play request");
                            }
                        }
                        else
                        {
                            LogError("no more free gamer slots");
                        }
                    } break;
                default:
                    break;
            }
        }
        packet = app_state->packet;

retry2:
        recv_len = recvfrom(app_state->socket, (char*)packet, app_state->packet_size, 0, (sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR)
        {
            int result = WSAGetLastError();
            if (result != WSAEWOULDBLOCK)
            {
                if (result == WSAECONNRESET)
                {
                    CloseConnection(sim, &client_addr);
                    goto retry2;
                }
                sprintf(temp_text, "recvfrom() error: %d", WSAGetLastError());
                LogError(temp_text);
            }
            return;
        }
    }
}

int32 WriteCurrentSimStateToPacket(Sim* sim, uint8* packet)
{
    assert(sim);
    assert(packet);
    auto o_packet = packet;

    WriteBin<int32>(&packet, 0);
    WriteBin<PacketType>(&packet, pk_SimState);
    WriteBin<int32>(&packet, ++sim->packet_cntr);
    WriteCurrentSimState(sim, &packet);
    int32 packet_length = packet - o_packet;
    packet = o_packet;
    WriteBin<int32>(&packet, packet_length);
    return packet_length;
}

void WriteSockets(WritePacketFunc write_packet)
{
    auto app_state = GetAppState();
    assert(app_state->socket);
    assert(write_packet);
    Sim* sim = &app_state->sim;
    assert(sim->is_active);

    if (sim->connections.count > 0)
    {
        int packet_length = write_packet(sim, app_state->packet);
        for (int i = 0; i < sim->connections.count; ++i)
        {
            Connection *c = sim->connections.ptr + i;
            int result = sendto(c->socket, (const char*)app_state->packet, packet_length, 0, (sockaddr *)&c->socket_addr, c->addr_len);
            if (result == SOCKET_ERROR)
            {
                sprintf(temp_text, "sendto() error: %d", WSAGetLastError());
                LogError(temp_text);
            }
            else
                app_state->bytes_sent_per_frame += result;
        }

        if (packet_length > app_state->packet_size_largest)
            app_state->packet_size_largest = packet_length;
    }
}
