#include "TSCustomPacket.h"
#include "TSPlayer.h"
#include "TSEvents.h"
#include "WorldPacket.h"
#include "CustomPacketChunk.h"
#include "Player.h"
#include "Log.h"

#include "TSMap.h"
#include "Map.h"
#include "TSBattleground.h"

TSPacketWrite::TSPacketWrite(CustomPacketWrite* write)
	// Custom deleter: CustomPacketWrite::Destroy() frees the chunk byte buffers,
	// but the heap-allocated CustomPacketWrite object header itself isn't freed
	// by Destroy() — we have to follow up with `delete`. The previous code
	// leaked the header (~24 bytes of metadata + the std::vector slots).
	: write(write, [](CustomPacketWrite* w) {
		if (w)
		{
			w->Destroy();
			delete w;
		}
	})
{}

TSPacketRead::TSPacketRead(CustomPacketRead* read)
	: read(read)
{}

void TSPacketWrite::SendToPlayer(TSPlayer player)
{
	if (!write)
	{
		TS_LOG_ERROR("tswow.api", "TSPacketWrite::SendToPlayer called on a null packet");
		return;
	}
	auto & arr = write->buildMessages();
	for (auto & chunk : arr)
	{
		WorldPacket packet(SERVER_TO_CLIENT_OPCODE, chunk.FullSize());
		packet.append((uint8_t*)chunk.Data(), chunk.FullSize());
		player.player->SendDirectMessage(&packet);
	}
	// Cleanup happens via shared_ptr deleter when this TSPacketWrite goes out
	// of scope; no manual Destroy() call here.
}

void TSPacketWrite::BroadcastMap(TSMap map, uint32_t teamOnly)
{
	if (!write)
	{
		TS_LOG_ERROR("tswow.api", "TSPacketWrite::BroadcastMap called on a null packet");
		return;
	}
	auto& arr = write->buildMessages();
	for (auto& chunk : arr)
	{
		WorldPacket packet(SERVER_TO_CLIENT_OPCODE, chunk.FullSize());
		packet.append((uint8_t*)chunk.Data(), chunk.FullSize());
		for (auto const& ref : map.map->GetPlayers())
		{
			Player* player = ref.GetSource();
#if TRINITY
			if (teamOnly == 0 || player->GetTeam() == teamOnly)
#endif
			{
				player->SendDirectMessage(&packet);
			}
		}
	}
}

void TSPacketWrite::BroadcastAround(TSWorldObject obj, float range, bool self)
{
	if (!write)
	{
		TS_LOG_ERROR("tswow.api", "TSPacketWrite::BroadcastAround called on a null packet");
		return;
	}
	auto& arr = write->buildMessages();
	for (auto& chunk : arr)
	{
		WorldPacket packet(SERVER_TO_CLIENT_OPCODE, chunk.FullSize());
		packet.append((uint8_t*)chunk.Data(), chunk.FullSize());
		obj.obj->SendMessageToSetInRange(&packet, range, self);
	}
}

TSServerBuffer::TSServerBuffer(TSPlayer player)
	: CustomPacketBuffer(
		  MIN_FRAGMENT_SIZE
		, BUFFER_QUOTA
		, MAX_FRAGMENT_SIZE
	)
	, m_player(player)
{
}

void TSServerBuffer::OnPacket(CustomPacketRead* value)
{
	// Expanded FIRE_ID macro because we need to reset the packet
	// reading head between every invocation.
	// Please do not change this to some auto-resetting macro abuse,
	// it would NOT be guaranteed to work in the long term.

	TSPacketRead read(value);
	opcode_t opcode = value->Opcode();

	auto& cbs = ts_events.CustomPacket.OnReceive_callbacks;
	for (auto const& cb : cbs.m_cxx_callbacks)
	{
			cb(opcode, read, m_player);
	}

	for (auto const& cb : cbs.m_lua_callbacks)
	{
			cb(opcode, read, m_player);
			value->Reset();
	}

	if (opcode < cbs.m_id_cxx_callbacks.size())
	{
			for (auto const& cb : cbs.m_id_cxx_callbacks[opcode])
			{
					cb(opcode, read, m_player);
					value->Reset();
			}
	}

	if (opcode < cbs.m_id_lua_callbacks.size())
	{
			for (auto const& cb : cbs.m_id_lua_callbacks[opcode])
			{
					cb(opcode, read, m_player);
					value->Reset();
			}
	}
}

void TSServerBuffer::OnError(CustomPacketResult error)
{
	m_player.player->GetSession()->KickPlayer("Custom packet error: "+std::to_string(uint32_t(error)));
}

TSPacketWrite CreateCustomPacket(
		opcode_t opcode
	, totalSize_t size
)
{
	// Refuse oversized allocations from livescripts. CustomPacketWrite
	// allocates `size` bytes up front; without this guard a malicious or
	// buggy script could request multiple GB and exhaust the worldserver.
	if (size > BUFFER_QUOTA)
	{
		TS_LOG_ERROR(
			  "tswow.api"
			, "CreateCustomPacket: requested size %u exceeds BUFFER_QUOTA %u"
			, size
			, BUFFER_QUOTA
		);
		return TSPacketWrite(nullptr);
	}
	// can we avoid heap allocation here?
	CustomPacketWrite* write = new CustomPacketWrite(
			opcode
		, MAX_FRAGMENT_SIZE
		, size
	);
	return TSPacketWrite(write);
}
