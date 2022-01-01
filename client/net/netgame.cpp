//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: netgame.cpp,v 1.60 2006/05/21 11:28:29 kyeman Exp $
//
//----------------------------------------------------------

#include <raknet/RakClient.h>
#include "../main.h"
#include "../game/util.h"
#include "../mod.h"

//INCAR_SYNC_DATA DebugSync;
//BOOL bDebugUpdate=FALSE;

static int iExceptMessageDisplayed=0;

int iVehiclesBench=0;
int iPlayersBench=0;
int iPicksupsBench=0;
int iMenuBench=0;
int iObjectBench=0;
//int iTextDrawBench=0;

static int iVehiclePoolProcessFlag=0;
static int iPickupPoolProcessFlag=0;

//----------------------------------------------------

BYTE GetPacketID(Packet *p)
{
	if (p==0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else {
		return (unsigned char) p->data[0];
	}
}

//----------------------------------------------------

CNetGame::CNetGame(PCHAR szHostOrIp, int iPort, 
				   PCHAR szPlayerName, PCHAR szPass)
{
	strcpy_s(m_szHostName, "San Andreas Multiplayer 0.3.7 R1");
	strncpy_s(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	m_iPort = iPort;

	UpdateDiscordPresence("Waiting to join...", m_szHostName);

	// Setup player pool
	m_pPlayerPool = new CPlayerPool();
	m_pPlayerPool->GetLocalPlayer()->SetName(szPlayerName);

	m_pVehiclePool = new CVehiclePool();
	m_pPickupPool  = new CPickupPool();
	m_pObjectPool	= new CObjectPool();
	m_pMenuPool = new CMenuPool();
	m_pTextDrawPool = new CTextDrawPool();
	m_pGangZonePool = new CGangZonePool();
	m_pActorPool = new CActorPool();
	m_pLabelPool = new CLabelPool();

	m_pRakClient = RakNetworkFactory::GetRakClientInterface();

	RegisterRPCs(m_pRakClient);
	RegisterScriptRPCs(m_pRakClient);	// Register server-side scripting RPCs.

	m_pRakClient->SetPassword(szPass);

	m_dwLastConnectAttempt = GetTickCount();
	m_iGameState = GAMESTATE_WAIT_CONNECT;
	
	m_iSpawnsAvailable = 0;
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather	= 10;
	m_fGravity = 0.008000000f;
	m_iDeathDropMoney = 0;
	m_bLanMode = false;
	m_byteHoldTime = 1;
	m_bUseCJWalk = false;
	m_bDisableEnterExits = false;
	m_bDisableVehMapIcons = false;
	m_fNameTagDrawDistance = 70.0f;
	m_bNameTagLOS = true;
	m_bAllowWeapons = true;
	m_bLimitGlobalMarkerRadius = false;
	m_iShowPlayerMarkers = true;
	m_bShowPlayerTags = true;
	m_bTirePopping = true;
	m_fGlobalMarkerRadius = 10000.0f;
	m_bManualVehicleEngineAndLight = false;

	m_WorldBounds[0] = m_WorldBounds[2] = 20000.0f;
	m_WorldBounds[1] = m_WorldBounds[3] = -20000.0f;

	int i;
	for (i = 0; i < MAX_MAP_ICON; i++) m_dwMapIcons[i] = NULL;

	m_byteFriendlyFire = 1;
	pGame->EnableClock(0); // Hide the clock by default
	pGame->EnableZoneNames(0);
	m_bManualVehicleEngineAndLight = false;
	m_bZoneNames = false;
	m_bInstagib = false;

	if (pChatWindow) {
		pChatWindow->AddDebugMessage("{FFFFFF}Open SAMP {B9C9BF}" SAMP_VERSION " {FFFFFF}Started");
		pChatWindow->AddInfoMessage("You are currently using an unstable version of the client, if you notice a bug, please write vk.com/nanseven");
	}

}

//----------------------------------------------------

CNetGame::~CNetGame()
{
	m_pRakClient->Disconnect(0);
	//UnRegisterRPCs(m_pRakClient);
	//UnRegisterScriptRPCs(m_pRakClient);	// Unregister server-side scripting RPCs.
	SAFE_DELETE(m_pRakClient);
	SAFE_DELETE(m_pPlayerPool);
	SAFE_DELETE(m_pVehiclePool);
	SAFE_DELETE(m_pPickupPool);
	SAFE_DELETE(m_pObjectPool);
	SAFE_DELETE(m_pMenuPool);
	SAFE_DELETE(m_pTextDrawPool);
	SAFE_DELETE(m_pGangZonePool);
	SAFE_DELETE(m_pActorPool);
	SAFE_DELETE(m_pLabelPool);
}

//----------------------------------------------------

void CNetGame::ShutdownForGameModeRestart()
{
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather	= 10;
	m_byteHoldTime = 1;
	m_bUseCJWalk = false;
	m_fGravity = (float)0.008000000;
	m_iDeathDropMoney = 0;
	pGame->SetGravity(m_fGravity);
	pGame->SetWantedLevel(0);
	pGame->EnableClock(0);
	m_bDisableEnterExits = false;
	m_bDisableVehMapIcons = false;
	m_fNameTagDrawDistance = 70.0f;
	m_bDisableVehicleCollision = false;

	for (PLAYERID bytePlayerID = 0; bytePlayerID < MAX_PLAYERS; bytePlayerID++) {
		CRemotePlayer* pPlayer = m_pPlayerPool->GetAt(bytePlayerID);
		if (pPlayer) {
			pPlayer->SetTeam(NO_TEAM);
			pPlayer->ResetAllSyncAttributes();
			pPlayer->SetVirtualWorld(0);
		}
	}
	m_pPlayerPool->GetLocalPlayer()->ResetAllSyncAttributes();
	m_pPlayerPool->GetLocalPlayer()->ToggleSpectating(false);
	GameResetLocalPlayerWeaponSkills();
	m_pPlayerPool->GetLocalPlayer()->SetVirtualWorld(0);
	m_iGameState = GAMESTATE_RESTARTING;

	pChatWindow->AddInfoMessage("Game mode restarting..");

	// Disable the ingame players and reset the vehicle pool.
	m_pPlayerPool->DeactivateAll();
	
	// Process the pool one last time
	m_pPlayerPool->Process();

	ResetVehiclePool();
	ResetPickupPool();
	ResetObjectPool();
	ResetMenuPool();
	ResetTextDrawPool();
	ResetGangZonePool();
	ResetActorPool();
	ResetLabelPool();

	if(pDeathWindow)
		pDeathWindow->ClearWindow();

	ResetMapIcons();
	pGame->ToggleCheckpoints(false);
	pGame->ToggleRaceCheckpoints(false);
	pGame->FindPlayerPed()->SetInterior(0);
	pGame->ResetLocalMoney();
	pGame->FindPlayerPed()->SetDead();
	pGame->FindPlayerPed()->SetArmour(0.0f);
	pGame->EnableZoneNames(0);
	m_bZoneNames = false;
	
	GameResetRadarColors();
}

//----------------------------------------------------

/*void CNetGame::InitGameLogic()
{
	//GameResetRadarColors();

	m_WorldBounds[0] = 20000.0f;
	m_WorldBounds[1] = -20000.0f;
	m_WorldBounds[2] = 20000.0f;
	m_WorldBounds[3] = -20000.0f;
}*/

//----------------------------------------------------

void CNetGame::Process()
{	
	UpdateNetwork();

	if (m_byteHoldTime)	{
		pGame->SetWorldTime(m_byteWorldTime, m_byteWorldMinute);
	}

	// Keep the weather fixed at m_byteWeather so it doesnt desync
	pGame->SetWorldWeather(m_byteWeather);

	// KEEP THE FOLLOWING ANIMS LOADED DURING THE NETGAME
	if(CGame::IsAnimationLoaded("PARACHUTE") == 0) CGame::RequestAnimation("PARACHUTE");
	if(CGame::IsAnimationLoaded("DANCING") == 0) CGame::RequestAnimation("DANCING");
	if(CGame::IsAnimationLoaded("GFUNK") == 0) CGame::RequestAnimation("GFUNK");
	if(CGame::IsAnimationLoaded("RUNNINGMAN") == 0)	CGame::RequestAnimation("RUNNINGMAN");
	if(CGame::IsAnimationLoaded("WOP") == 0) CGame::RequestAnimation("WOP");
	if(CGame::IsAnimationLoaded("STRIP") == 0) CGame::RequestAnimation("STRIP");
	if(CGame::IsAnimationLoaded("PAULNMAC") == 0) CGame::RequestAnimation("PAULNMAC");
				
	if(!CGame::IsModelLoaded(OBJECT_PARACHUTE)) {
		CGame::RequestModel(OBJECT_PARACHUTE);
	}

	// keep the throwable weapon models loaded
	if (!CGame::IsModelLoaded(WEAPON_MODEL_TEARGAS))
		CGame::RequestModel(WEAPON_MODEL_TEARGAS);
	if (!CGame::IsModelLoaded(WEAPON_MODEL_GRENADE))
		CGame::RequestModel(WEAPON_MODEL_GRENADE);
	if (!CGame::IsModelLoaded(WEAPON_MODEL_MOLTOV))
		CGame::RequestModel(WEAPON_MODEL_MOLTOV);

	// cellphone
	if (!CGame::IsModelLoaded(330)) CGame::RequestModel(330);

	if(GetGameState() == GAMESTATE_CONNECTED) {

		DWORD dwStartTick = GetTickCount();

		if(m_pPlayerPool) m_pPlayerPool->Process();
		iPlayersBench += GetTickCount() - dwStartTick;

		if(m_pVehiclePool && iVehiclePoolProcessFlag > 5) {
			dwStartTick = GetTickCount();

			try { m_pVehiclePool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing vehicle pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iVehiclesBench += GetTickCount() - dwStartTick;
			iVehiclePoolProcessFlag = 0;
		} else {
			iVehiclePoolProcessFlag++;
		}
			
		if(m_pPickupPool && iPickupPoolProcessFlag > 10) {

			dwStartTick = GetTickCount();

			try { m_pPickupPool->Process(); }
			catch(...) {
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing pickup pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iPicksupsBench += GetTickCount() - dwStartTick;
			iPickupPoolProcessFlag = 0;
		}
		else
		{
			iPickupPoolProcessFlag++;
		}

		if(m_pObjectPool) {
			dwStartTick = GetTickCount();
			try { m_pObjectPool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing object pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iObjectBench += GetTickCount() - dwStartTick;
		}

		if(m_pMenuPool) {
			dwStartTick = GetTickCount();
			try { m_pMenuPool->Process(); }
			catch(...) { 
				if(!iExceptMessageDisplayed) {				
					pChatWindow->AddDebugMessage("Warning: Error processing menu pool"); 
					iExceptMessageDisplayed++;
				}
			}
			iMenuBench += GetTickCount() - dwStartTick;
		}	
	}
	else {
		CPlayerPed* pLocalPed = pGame->FindPlayerPed();
		if(pLocalPed->IsInVehicle()) {
			pLocalPed->RemoveFromVehicleAndPutAt(1093.4f, -2036.5f, 82.710602f);
		} else {
			pLocalPed->TeleportTo(1133.0504f, -2038.4034f, 69.1f);
		}
		pGame->GetCamera()->SetPosition(1093.0f, -2036.0f, 90.0f,0.0f,0.0f,0.0f);
		pGame->GetCamera()->LookAtPoint(384.0f, -1557.0f, 20.0f,2);
		pLocalPed->TogglePlayerControllable(0);
		pGame->SetWorldWeather(1);
		pGame->DisplayHud(false);
	}

	if( GetGameState() == GAMESTATE_WAIT_CONNECT && 
		(GetTickCount() - m_dwLastConnectAttempt) > 3000) 
	{
		if(pChatWindow) pChatWindow->AddDebugMessage("Connecting to %s:%d...",m_szHostOrIp,m_iPort);
		m_pRakClient->Connect(m_szHostOrIp,m_iPort,0,0,10);
		m_dwLastConnectAttempt = GetTickCount();
		SetGameState(GAMESTATE_CONNECTING);
	}
}

//----------------------------------------------------
// UPDATE NETWORK
//----------------------------------------------------

void CNetGame::UpdateNetwork()
{
	unsigned char packetIdentifier;
	Packet* pkt;

	while((pkt = m_pRakClient->Receive()))
	{
		if ((unsigned char)pkt->data[0] == ID_TIMESTAMP)
		{
			if (pkt->length > sizeof(unsigned char) + sizeof(unsigned int))
				packetIdentifier = (unsigned char)pkt->data[sizeof(unsigned char) + sizeof(unsigned int)];
			else
				return;
		}
		else
			packetIdentifier = (unsigned char)pkt->data[0];
		//packetIdentifier = GetPacketID(pkt);

		pChatWindow->AddDebugMessage("Receive packet: %d", pkt->data[0]);

		switch(packetIdentifier)
		{
		case ID_CONNECTION_BANNED:
			Packet_ConnectionBanned(pkt);
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Packet_NoFreeIncomingConnections(pkt);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			Packet_DisconnectionNotification(pkt);
			break;
		case ID_CONNECTION_LOST:
			Packet_ConnectionLost(pkt);
			break;
		case ID_INVALID_PASSWORD:
			Packet_InvalidPassword(pkt);
			break;
		case ID_CONNECTION_ATTEMPT_FAILED: 
			Packet_ConnectAttemptFailed(pkt); 
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);
			break;
		case ID_AUTH_KEY:
			Packet_AuthKey(pkt);
			break;
		case ID_PLAYER_SYNC:
			Packet_PlayerSync(pkt);
			break;
		case ID_VEHICLE_SYNC:
			Packet_VehicleSync(pkt);
			break;
		case ID_PASSENGER_SYNC:
			Packet_PassengerSync(pkt);
			break;
		case ID_AIM_SYNC:
			Packet_AimSync(pkt);
			break;
		case ID_TRAILER_SYNC:
			Packet_TrailerSync(pkt);
			break;
		}

		m_pRakClient->DeallocatePacket(pkt);		
	}

}

//----------------------------------------------------
// PACKET HANDLERS INTERNAL
//----------------------------------------------------

void CNetGame::Packet_PlayerSync(Packet *p)
{
	CRemotePlayer* pPlayer;
	RakNet::BitStream bsPlayerSync((unsigned char*)p->data, p->length, false);
	ONFOOT_SYNC_DATA ofSync;
	uint8_t bytePacketID = 0;
	PLAYERID playerId;
	
	bool bHasLR, bHasUD;
	bool bHasVehicleSurfingInfo;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&ofSync,0,sizeof(ONFOOT_SYNC_DATA));

	bsPlayerSync.Read(bytePacketID);
	bsPlayerSync.Read(playerId);

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("[RakLogger] <- OnFoot Packet. Player: %d", playerId);
#endif

	//bsPlayerSync.Read((PCHAR)&ofSync,sizeof(ONFOOT_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if (bHasLR) bsPlayerSync.Read(ofSync.lrAnalog);
	
	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if (bHasUD) bsPlayerSync.Read(ofSync.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(ofSync.wKeys);

	// VECTOR POS
	bsPlayerSync.Read((char*)&ofSync.vecPos, sizeof(VECTOR));

	// ROTATION
	float tw, tx, ty, tz;
	bsPlayerSync.ReadNormQuat(tw, tx, ty, tz);
	/**ofSync.quat.w = tw;
	ofSync.quat.x = tx;
	ofSync.quat.y = ty;
	ofSync.quat.z = tz;
	
	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) ofSync.byteArmour = 100;
	else if(byteArmTemp == 0) ofSync.byteArmour = 0;
	else ofSync.byteArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) ofSync.byteHealth = 100;
	else if(byteHlTemp == 0) ofSync.byteHealth = 0;
	else ofSync.byteHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	bsPlayerSync.Read(ofSync.byteCurrentWeapon);

	// Special Action
	bsPlayerSync.Read(ofSync.byteSpecialAction);
	
	// READ MOVESPEED VECTORS
	bsPlayerSync.Read(bMoveSpeedX);
	if(bMoveSpeedX) bsPlayerSync.Read(ofSync.vecMoveSpeed.X);
	else ofSync.vecMoveSpeed.X = 0.0f;

	bsPlayerSync.Read(bMoveSpeedY);
	if(bMoveSpeedY) bsPlayerSync.Read(ofSync.vecMoveSpeed.Y);
	else ofSync.vecMoveSpeed.Y = 0.0f;

	bsPlayerSync.Read(bMoveSpeedZ);
	if(bMoveSpeedZ) bsPlayerSync.Read(ofSync.vecMoveSpeed.Z);
	else ofSync.vecMoveSpeed.Z = 0.0f;

	bsPlayerSync.Read(bHasVehicleSurfingInfo);
	if(bHasVehicleSurfingInfo) {
		bsPlayerSync.Read(ofSync.SurfVehicleId);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.X);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.Y);
		bsPlayerSync.Read(ofSync.vecSurfOffsets.Z);
	} else {
		ofSync.SurfVehicleId = INVALID_VEHICLE_ID;
	}*/

	pPlayer = GetPlayerPool()->GetAt(playerId);

	if(pPlayer) {
		pPlayer->StoreOnFootFullSyncData(&ofSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_AimSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsAimSync((unsigned char*)p->data, p->length, false);
	AIM_SYNC_DATA aimSync;
	BYTE bytePlayerID=0;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsAimSync.IgnoreBits(8);
	bsAimSync.Read(bytePlayerID);
	bsAimSync.Read((PCHAR)&aimSync,sizeof(AIM_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer) {
		pPlayer->UpdateAimFromSyncData(&aimSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_VehicleSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSync((unsigned char*)p->data, p->length, false);
	BYTE		bytePlayerID=0;
	INCAR_SYNC_DATA icSync;

	bool bSiren,bLandingGear;
	bool bHydra,bTrain,bTrailer;
	bool bTire;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&icSync,0,sizeof(INCAR_SYNC_DATA));

	bsSync.IgnoreBits(8);
	bsSync.Read(bytePlayerID);
	bsSync.Read(icSync.VehicleID);

	//bsSync.Read((PCHAR)&icSync,sizeof(INCAR_SYNC_DATA));

	// KEYS
	bsSync.Read(icSync.lrAnalog);
	bsSync.Read(icSync.udAnalog);
	bsSync.Read(icSync.wKeys);

	// ROLL / DIRECTION / POSITION / MOVE SPEED
	bsSync.Read((char*)&icSync.cvecRoll,sizeof(C_VECTOR1));
	bsSync.Read((char*)&icSync.cvecDirection,sizeof(C_VECTOR1));
	bsSync.Read((char*)&icSync.vecPos,sizeof(VECTOR));
	bsSync.Read((char*)&icSync.vecMoveSpeed,sizeof(VECTOR));

	// VEHICLE HEALTH
	WORD wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	icSync.fCarHealth = (float)wTempVehicleHealth;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) icSync.bytePlayerArmour = 100;
	else if(byteArmTemp == 0) icSync.bytePlayerArmour = 0;
	else icSync.bytePlayerArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) icSync.bytePlayerHealth = 100;
	else if(byteHlTemp == 0) icSync.bytePlayerHealth = 0;
	else icSync.bytePlayerHealth = byteHlTemp * 7;

	// CURRENT WEAPON
	bsSync.Read(icSync.byteCurrentWeapon);
	
	// SIREN
	bsSync.Read(bSiren);
	if(bSiren) icSync.byteSirenOn = 1;

	// LANDING GEAR
	bsSync.Read(bLandingGear);
	if(bLandingGear) icSync.byteLandingGearState = 1;

	if (m_bTirePopping) {
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[0] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[1] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[2] = 1;
		bsSync.Read(bTire);
		if (bTire) icSync.byteTires[3] = 1;
	}

	// HYDRA SPECIAL
	bsSync.Read(bHydra);
	if(bHydra) bsSync.Read(icSync.dwHydraThrustAngle);

	// TRAIN SPECIAL
	bsSync.Read(bTrain);
	if(bTrain) bsSync.Read(icSync.fTrainSpeed);

	// TRAILER ID
	bsSync.Read(bTrailer);
	if(bTrailer) bsSync.Read(icSync.TrailerID);
	
	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer)	{
		pPlayer->StoreInCarFullSyncData(&icSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_PassengerSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPassengerSync((unsigned char*)p->data, p->length, false);
	BYTE		bytePlayerID=0;
	PASSENGER_SYNC_DATA psSync;

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	bsPassengerSync.IgnoreBits(8);
	bsPassengerSync.Read(bytePlayerID);
	bsPassengerSync.Read((PCHAR)&psSync,sizeof(PASSENGER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	//OutputDebugString("Getting Passenger Packets");

	if(pPlayer)	{
		pPlayer->StorePassengerFullSyncData(&psSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_TrailerSync(Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSpectatorSync((unsigned char*)p->data, p->length, false);

	if(GetGameState() != GAMESTATE_CONNECTED) return;

	BYTE bytePlayerID=0;
	TRAILER_SYNC_DATA trSync;
	
	bsSpectatorSync.IgnoreBits(8);
	bsSpectatorSync.Read(bytePlayerID);
	bsSpectatorSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));

	pPlayer = GetPlayerPool()->GetAt(bytePlayerID);

	if(pPlayer)	{
	    pPlayer->StoreTrailerFullSyncData(&trSync);
	}
}

//----------------------------------------------------

void CNetGame::Packet_RSAPublicKeyMismatch(Packet* packet)
{
	pChatWindow->AddDebugMessage("Failed to initialize encryption.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionBanned(Packet* packet)
{
	pChatWindow->AddDebugMessage("You're banned from this server.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionRequestAccepted(Packet* packet)
{
	pChatWindow->AddDebugMessage("Server has accepted the connection.");
}

//----------------------------------------------------

void CNetGame::Packet_NoFreeIncomingConnections(Packet* packet)
{
	pChatWindow->AddDebugMessage("The server is full. Retrying...");
	SetGameState(GAMESTATE_WAIT_CONNECT);	
}

//----------------------------------------------------

void CNetGame::Packet_DisconnectionNotification(Packet* packet)
{
	pChatWindow->AddDebugMessage("Server closed the connection.");
	m_pRakClient->Disconnect(0);
}

//----------------------------------------------------

const static BYTE code_from_CAnimManager_AddAnimation[20] = {
	0xFF, 0x25, 0x34, 0x39, // gta_sa.exe + 0x4D3AA0
	0x4D, 0x00, 0x90, 0x90, // gta_sa.exe + 0x4D3AA4
	0x90, 0x90, 0x56, 0x57, // gta_sa.exe + 0x4D3AAC
	0x50, 0x8B, 0x44, 0x24, // gta_sa.exe + 0x4D3AA8
	0x14, 0x8D, 0x0C, 0x80  // gta_sa.exe + 0x4D3AB0
};

const static BYTE auth_hash_transform_table[100] = {
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
	0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x08, 0x06, 0x00, 0x00, 0x00, 0xE4, 0xB5, 0xB7, 0x0A, 0x00, 0x00, 0x00,
	0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00, 0x00, 0x0B,
	0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41,
	0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8E, 0x7C, 0xFB, 0x51, 0x93, 0x00, 0x00,
	0x00, 0x20, 0x63, 0x48, 0x52, 0x4D, 0x00, 0x00, 0x7A, 0x25, 0x00, 0x00,
	0x80, 0x83, 0x00, 0x00, 0xF9, 0xFF, 0x00, 0x00, 0x80, 0xE9, 0x00, 0x00,
	0x75, 0x30, 0x00, 0x00
};

#define endian_swap8(x) (x)
#define endian_swap16(x) ((x>>8) | (x<<8))
#define endian_swap32(x) ((x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24))
#define endian_swap64(x) ((x>>56) | ((x<<40) & 0x00FF000000000000) | \
		((x<<24) & 0x0000FF0000000000) | ((x<<8)  & 0x000000FF00000000) | \
		((x>>8)  & 0x00000000FF000000) | ((x>>24) & 0x0000000000FF0000) | \
        ((x>>40) & 0x000000000000FF00) | (x<<56))

#define ROTL(value, shift) ((value << shift) | (value >> (sizeof(value)*8 - shift)))
#define ROTR(value, shift) ((value >> shift) | (value << (sizeof(value)*8 - shift)))
#define swap(x,y,T) {T tmp = x; x = y; y = tmp;}

void SHA1(char* message, unsigned long* out)
{
	unsigned long h0 = 0x67452301;
	unsigned long h1 = 0xEFCDAB89;
	unsigned long h2 = 0x98BADCFE;
	unsigned long h3 = 0x10325476;
	unsigned long h4 = 0xC3D2E1F0;

	unsigned long len = 0;
	unsigned long long bitlen = 0;

	while (message[len])
	{
		len++;
		bitlen += 8;
	}

	unsigned long complement = (55 - (len % 56)) + 8 * (((len + 8) / 64));
	unsigned long newlen = len + complement + 8 + 1;
	char* pMessage = new char[newlen];
	if (!pMessage)
		return;

	memcpy(pMessage, message, len);
	pMessage[len] = -128;
	memset(pMessage + len + 1, 0, complement);

	*(unsigned long long*)& pMessage[len + 1 + complement] = endian_swap64(bitlen);

	unsigned long chunks = newlen / 64;
	unsigned long w[80];

	for (unsigned long x = 0; x < chunks; x++)
	{
		for (unsigned long i = 0; i < 16; i++)
			w[i] = endian_swap32(*(unsigned long*)(&pMessage[x * 64 + i * 4]));

		memset(&w[16], 0, 64 * 4);

		for (unsigned long i = 16; i <= 79; i++)
			w[i] = ROTL((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]), 1);

		unsigned long a = h0;
		unsigned long b = h1;
		unsigned long c = h2;
		unsigned long d = h3;
		unsigned long e = h4;

		for (unsigned long i = 0; i <= 79; i++)
		{
			unsigned long f;
			unsigned long k;

			if (0 <= i && i <= 19)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (20 <= i && i <= 39)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (40 <= i && i <= 59)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else if (60 <= i && i <= 79)
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			unsigned long temp = (ROTL(a, 5) + f + e + k + w[i]) & 0xFFFFFFFF;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = temp;
		}

		h0 = (h0 + a) & 0xFFFFFFFF;
		h1 = (h1 + b) & 0xFFFFFFFF;
		h2 = (h2 + c) & 0xFFFFFFFF;
		h3 = (h3 + d) & 0xFFFFFFFF;
		h4 = (h4 + e) & 0xFFFFFFFF;
	}

	delete[] pMessage;

	out[0] = h0;
	out[1] = h1;
	out[2] = h2;
	out[3] = h3;
	out[4] = h4;
}

void SHA1(char* message, char buf[64])
{
	if (!buf) return;
	unsigned long out[5];
	SHA1(message, out);
	sprintf_s(buf, 64, "%.8X%.8X%.8X%.8X%.8X", out[0], out[1], out[2], out[3], out[4]);
}

char samp_sub_100517E0(BYTE a1)
{
	char result = a1 + '0';

	if (a1 + '0' > '9')
	{
		result = a1 + '7';
	}

	return result;
}



BYTE transform_auth_sha1(BYTE value, BYTE xor)
{
	BYTE result = value;

	for (BYTE i = 0; i < 100; i++)
	{
		result = result ^ auth_hash_transform_table[i] ^ xor;
	}

	return result;
}

void auth_stringify(char* out, PBYTE hash)
{
	BYTE i = 0;
	PBYTE j = hash;

	do
	{
		out[i] = samp_sub_100517E0(*j >> 4); i++;
		out[i] = samp_sub_100517E0(*j & 0xF); i++;

		j++;
	} while (i < 40);

	out[i] = '\0';
}

void gen_auth_key(char buf[260], char* auth_in)
{
	char message[260];
	if (!auth_in) return;
	sprintf_s(message, 260, "%s", auth_in);

	unsigned long out[5];
	BYTE* pb_out = (PBYTE)&out;

	SHA1(message, out);

	for (BYTE i = 0; i < 5; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x2F); }
	for (BYTE i = 5; i < 10; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x45); }
	for (BYTE i = 10; i < 15; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0x6F); }
	for (BYTE i = 15; i < 20; i++) { pb_out[i] = transform_auth_sha1(pb_out[i], 0xDB); }
	for (BYTE i = 0; i < 20; i++) { pb_out[i] ^= code_from_CAnimManager_AddAnimation[i]; }

	auth_stringify(buf, pb_out);
}

void CNetGame::Packet_AuthKey(Packet* packet)
{
	RakNet::BitStream bsAuth((unsigned char*)packet->data, packet->length, false);

	BYTE byteAuthLen;
	char szAuth[260];

	bsAuth.IgnoreBits(8); // ID_AUTH_KEY
	bsAuth.Read(byteAuthLen);
	bsAuth.Read(szAuth, byteAuthLen);
	szAuth[byteAuthLen] = '\0';

	char szAuthKey[260];

	gen_auth_key(szAuthKey, szAuth);

	RakNet::BitStream bsKey;
	BYTE byteAuthKeyLen = (BYTE)strlen(szAuthKey);

	bsKey.Write((BYTE)ID_AUTH_KEY);
	bsKey.Write((BYTE)byteAuthKeyLen);
	bsKey.Write(szAuthKey, byteAuthKeyLen);

	m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, NULL);

	//pChatWindow->AddDebugMessage("[AUTH] %s -> %s", szAuth, szAuthKey);
}

void CNetGame::Packet_ConnectionLost(Packet* packet)
{
	pChatWindow->AddDebugMessage("Lost connection to the server. Reconnecting..");
	ShutdownForGameModeRestart();
    SetGameState(GAMESTATE_WAIT_CONNECT);	
}

//----------------------------------------------------

void CNetGame::Packet_InvalidPassword(Packet* packet)
{
	pChatWindow->AddDebugMessage("Wrong server password.");
	m_pRakClient->Disconnect(0);
}

//----------------------------------------------------

void CNetGame::Packet_ModifiedPacket(Packet* packet)
{
#ifdef _DEBUG
	char szBuffer[256];
	/*sprintf_s(szBuffer, "Packet was modified, sent by id: %d, ip: %s",
					(unsigned int)packet->playerIndex, packet->playerId.ToString());
	pChatWindow->AddDebugMessage(szBuffer);*/
	//m_pRakClient->Disconnect(0);
#endif
}

//----------------------------------------------------
// RST

void CNetGame::Packet_ConnectAttemptFailed(Packet* packet)
{
	pChatWindow->AddDebugMessage("The server didn't respond. Retrying..");
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

//----------------------------------------------------
// Connection Success

void CNetGame::Packet_ConnectionSucceeded(Packet *p)
{
	if (pChatWindow) {
		pChatWindow->AddDebugMessage("Connected. Joining the game...");
	}
	RakNet::BitStream bsReturnParams((unsigned char*)p->data, p->length, false);

	unsigned int uiChallenge;
	PLAYERID MyPlayerID;
	CLocalPlayer* pPlayer = m_pPlayerPool->GetLocalPlayer();
	SetGameState(GAMESTATE_AWAIT_JOIN);

	bsReturnParams.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
	bsReturnParams.IgnoreBits(32); // binaryAddress
	bsReturnParams.IgnoreBits(16); // port

	bsReturnParams.Read(MyPlayerID);
	bsReturnParams.Read(uiChallenge);

	m_pPlayerPool->SetLocalPlayerID(MyPlayerID);

	int iVersion = NETGAME_VERSION;
	char byteMod = 0x01;
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;

	char byteAuthBSLen = (char)strlen("15121F6F18550C00AC4B4F8A167D0379BB0ACA99043");
	char byteNameLen = (char)strlen(pPlayer->GetName());
	char byteClientverLen = (char)strlen("0.3.7");

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(pPlayer->GetName(), byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write("15121F6F18550C00AC4B4F8A167D0379BB0ACA99043", byteAuthBSLen);
	bsSend.Write(byteClientverLen);
	bsSend.Write("0.3.7", byteClientverLen);
	m_pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

//----------------------------------------------------

void CNetGame::UpdatePlayerPings()
{
	static DWORD dwLastUpdateTick = 0;

	if ((GetTickCount() - dwLastUpdateTick) > RPC_PING_UPDATE_TIME) {
		dwLastUpdateTick = GetTickCount();
		m_pRakClient->RPC(&RPC_UpdateScoresPingsIPs, NULL, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	}
}

//----------------------------------------------------

void CNetGame::ResetVehiclePool()
{
	if(m_pVehiclePool) {
		delete m_pVehiclePool;
	}
	m_pVehiclePool = new CVehiclePool();
}

//----------------------------------------------------

void CNetGame::ResetPlayerPool()
{
	if(m_pPlayerPool) {
		delete m_pPlayerPool;
	}
	m_pPlayerPool = new CPlayerPool();
}

//----------------------------------------------------

void CNetGame::ResetPickupPool()
{
	if(m_pPickupPool) {
		delete m_pPickupPool;
	}
	m_pPickupPool = new CPickupPool();
}

//----------------------------------------------------

void CNetGame::ResetMenuPool()
{
	if(m_pMenuPool) {
		delete m_pMenuPool;
	}
	m_pMenuPool = new CMenuPool();
}

//----------------------------------------------------

void CNetGame::ResetTextDrawPool()
{
	if(m_pTextDrawPool) {
		delete m_pTextDrawPool;
	}
	m_pTextDrawPool = new CTextDrawPool();
}

//----------------------------------------------------

void CNetGame::ResetObjectPool()
{
	if(m_pObjectPool) {
		delete m_pObjectPool;
	}
	m_pObjectPool = new CObjectPool();
}

//----------------------------------------------------

void CNetGame::ResetGangZonePool()
{
	if(m_pGangZonePool) {
		delete m_pGangZonePool;
	}
	m_pGangZonePool = new CGangZonePool();
}

//----------------------------------------------------

void CNetGame::ResetActorPool()
{
	if (m_pActorPool)
		delete m_pActorPool;

	m_pActorPool = new CActorPool();
}

//----------------------------------------------------

void CNetGame::ResetLabelPool()
{
	if (m_pLabelPool) {
		delete m_pLabelPool;
	}
	m_pLabelPool = new CLabelPool();
}


//-----------------------------------------------------------
// Puts a personal marker using any of the radar icons on the map

void CNetGame::SetMapIcon(BYTE byteIndex, float fX, float fY, float fZ, BYTE byteIcon, DWORD dwColor, BYTE byteStyle)
{
	if (byteIndex >= MAX_MAP_ICON) return;
	if (m_dwMapIcons[byteIndex] != NULL) DisableMapIcon(byteIndex);
	//ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, byteIcon, &m_dwMapIcon);
	m_dwMapIcons[byteIndex] = pGame->CreateRadarMarkerIcon(byteIcon, fX, fY, fZ, dwColor, byteStyle);
}

//-----------------------------------------------------------
// Removes the Map Icon

void CNetGame::DisableMapIcon(BYTE byteIndex)
{
	if (byteIndex >= MAX_MAP_ICON) return;
	ScriptCommand(&disable_marker, m_dwMapIcons[byteIndex]);
	m_dwMapIcons[byteIndex] = NULL;
}

//----------------------------------------------------

void CNetGame::ResetMapIcons()
{
	BYTE i;
	for (i = 0; i < MAX_MAP_ICON; i++)
	{
		if (m_dwMapIcons[i] != NULL) DisableMapIcon(i);
	}
}

//----------------------------------------------------
// EOF
