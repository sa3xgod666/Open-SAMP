/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		scriptrpc.cpp
	desc:
		Scripting RPCs.

	Version: $Id: scriptrpc.cpp,v 1.36 2006/05/08 14:31:50 kyeman Exp $
*/

#include "../main.h"

using namespace RakNet;

//----------------------------------------------------

void ScrSetSpawnInfo(RPCParameters *rpcParams)
{
	PLAYER_SPAWN_INFO SpawnInfo;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read((PCHAR)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	/*
	pChatWindow->AddDebugMessage("Got Spawn Info: %i %i %f %f %f %f %i %i %i %i %i %i",
								SpawnInfo.byteTeam,
								SpawnInfo.iSkin,
								SpawnInfo.vecPos.X,
								SpawnInfo.vecPos.Y,
								SpawnInfo.vecPos.Z,
								SpawnInfo.fRotation,
								SpawnInfo.iSpawnWeapons[0],
								SpawnInfo.iSpawnWeaponsAmmo[0],
								SpawnInfo.iSpawnWeapons[1],
								SpawnInfo.iSpawnWeaponsAmmo[1],
								SpawnInfo.iSpawnWeapons[2],
								SpawnInfo.iSpawnWeaponsAmmo[2]);*/

	pPlayerPool->GetLocalPlayer()->SetSpawnInfo(&SpawnInfo);

	//pPlayerPool->GetAt(bytePlayerID)->SetTeam(byteTeam);
}

//----------------------------------------------------

void ScrSetPlayerTeam(RPCParameters *rpcParams)
{
	BYTE bytePlayerID;
	BYTE byteTeam;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(bytePlayerID);
	bsData.Read(byteTeam);
	
	if (bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->SetTeam(byteTeam);
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(bytePlayerID);
		if(pPlayer) pPlayer->SetTeam(byteTeam);
	}
}

//----------------------------------------------------

void ScrSetPlayerName(RPCParameters *rpcParams)
{
	BYTE bytePlayerID;
	BYTE byteNickLen;
	char szNewName[MAX_PLAYER_NAME+1];
	BYTE byteSuccess;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(bytePlayerID);
	bsData.Read(byteNickLen);

	if(byteNickLen > MAX_PLAYER_NAME) return;

	bsData.Read(szNewName, byteNickLen);
	szNewName[byteNickLen] = '\0';

	// TODO: Remove this
	// Why would you even send packet from server to tell the client it was a success,
	// a.k.a. when the nick is not in use?
	bsData.Read(byteSuccess);
	
	if (pPlayerPool->GetLocalPlayerID() == bytePlayerID) {
		CLocalPlayer* pPlayer = pPlayerPool->GetLocalPlayer();

		if (pDeathWindow)
			pDeathWindow->ChangeNick((PCHAR)pPlayer->GetName(), szNewName);

		pPlayer->SetName(szNewName);
	} else {
		CRemotePlayer* pPlayer = pPlayerPool->GetAt(bytePlayerID);
		if (pPlayer != NULL) {
			if (pDeathWindow)
				pDeathWindow->ChangeNick((PCHAR)pPlayer->GetName(), szNewName);

			pPlayer->SetName(szNewName);
		}
	}
}

void ScrSetPlayerSkin(RPCParameters *rpcParams)
{
	int iPlayerID;
	unsigned int uiSkin;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bsData.Read(iPlayerID);
	bsData.Read(uiSkin);

	if (iPlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->GetPlayerPed()->SetModelIndex(uiSkin);
	}
	else {
		if(pPlayerPool->GetSlotState(iPlayerID) && pPlayerPool->GetAt(iPlayerID)->GetPlayerPed()) {
			pPlayerPool->GetAt(iPlayerID)->GetPlayerPed()->SetModelIndex(uiSkin);
		}
	}
}

//----------------------------------------------------

void ScrSetPlayerPos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X,vecPos.Y,vecPos.Z);
}

//----------------------------------------------------

void ScrSetPlayerPosFindZ(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	VECTOR vecPos;

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	vecPos.Z = pGame->FindGroundZForCoord(vecPos.X, vecPos.Y, vecPos.Z);
	vecPos.Z += 1.5f;

	pLocalPlayer->GetPlayerPed()->TeleportTo(vecPos.X, vecPos.Y, vecPos.Z);
}

//----------------------------------------------------

void ScrSetPlayerHealth(RPCParameters *rpcParams)
{
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fHealth;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(fHealth);

	//pChatWindow->AddDebugMessage("Setting your health to: %f", fHealth);
	pLocalPlayer->GetPlayerPed()->SetHealth(fHealth);
}

//----------------------------------------------------

void ScrPutPlayerInVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	VEHICLEID vehicleid;
	BYTE seatid;
	bsData.Read(vehicleid);
	bsData.Read(seatid);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	int iVehicleIndex = pNetGame->GetVehiclePool()->FindGtaIDFromID(vehicleid);
	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(vehicleid);

	if(iVehicleIndex && pVehicle) {
		CPlayerPed *pPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		if(pPed) {
			pPed->Add(); pVehicle->Add();
			pPed->PutDirectlyInVehicle(iVehicleIndex, seatid);
		}
	}
}

//----------------------------------------------------

void ScrRemovePlayerFromVehicle(RPCParameters *rpcParams)
{
	//RakNet::BitStream bsData(rpcParams);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	pPlayerPool->GetLocalPlayer()->GetPlayerPed()->ExitCurrentVehicle();
}

//----------------------------------------------------

void ScrSetPlayerColor(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	BYTE bytePlayerID;
	DWORD dwColor;

	bsData.Read(bytePlayerID);
	bsData.Read(dwColor);

	if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
		pPlayerPool->GetLocalPlayer()->SetPlayerColor(dwColor);
	} else {
		CRemotePlayer *pPlayer = pPlayerPool->GetAt(bytePlayerID);
		if(pPlayer)	pPlayer->SetPlayerColor(dwColor);
	}
}

//----------------------------------------------------

void ScrDisplayGameText(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	char szMessage[512];
	int iType;
	int iTime;
	int iLength;

	bsData.Read(iType);
	bsData.Read(iTime);
	bsData.Read(iLength);

	if(iLength > 512) return;

	bsData.Read(szMessage,iLength);
	szMessage[iLength] = '\0';

	pGame->DisplayGameText(szMessage,iTime,iType);
}

//----------------------------------------------------

void ScrSetInterior(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteInterior;
	bsData.Read(byteInterior);
	
	//pChatWindow->AddDebugMessage("ScrSetInterior(%u)",byteInterior);

	pGame->FindPlayerPed()->SetInterior(byteInterior);	
}

//----------------------------------------------------

void ScrSetCameraPos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);
	VECTOR vecPos;

	if(in.Read(vecPos))
		pGame->GetCamera()->SetPosition(vecPos.X, vecPos.Y,
			vecPos.Z, 0.0f, 0.0f, 0.0f);
}

//----------------------------------------------------

void ScrSetCameraLookAt(RPCParameters *rpcParams)
{
	if (rpcParams->numberOfBitsOfData == 104) {
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
		VECTOR vecPos;
		//unsigned char ucStyle;

		bsData.Read(vecPos.X);
		bsData.Read(vecPos.Y);
		bsData.Read(vecPos.Z);
		//bsData.Read(ucStyle);

		/*if (ucStyle < 1 || ucStyle > 2)
			ucStyle = 2;*/

		pGame->GetCamera()->LookAtPoint(vecPos.X, vecPos.Y, vecPos.Z, 2);
	}
}

//----------------------------------------------------

void ScrSetVehiclePos(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	VEHICLEID VehicleId;
	float fX, fY, fZ;
	bsData.Read(VehicleId);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	if(pNetGame && pNetGame->GetVehiclePool()->GetSlotState(VehicleId)) {
		pNetGame->GetVehiclePool()->GetAt(VehicleId)->TeleportTo(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrSetVehicleZAngle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	VEHICLEID VehicleId;
	float fZAngle;
	bsData.Read(VehicleId);
	bsData.Read(fZAngle);

	ScriptCommand(&set_car_z_angle, pNetGame->GetVehiclePool()->GetAt(VehicleId)->m_dwGTAId, fZAngle);
}

//----------------------------------------------------

void ScrVehicleParams(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID VehicleID;
	BYTE byteObjectiveVehicle;
	BYTE byteDoorsLocked;

	bsData.Read(VehicleID);
	bsData.Read(byteObjectiveVehicle);
	bsData.Read(byteDoorsLocked);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->AssignSpecialParamsToVehicle(VehicleID,byteObjectiveVehicle,byteDoorsLocked);

}

//----------------------------------------------------

void ScrLinkVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID VehicleID;
	BYTE byteInterior;

	bsData.Read(VehicleID);
	bsData.Read(byteInterior);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	pVehiclePool->LinkToInterior(VehicleID, (int)byteInterior);
}

//----------------------------------------------------

void ScrSetCameraBehindPlayer(RPCParameters *rpcParams)
{
	pGame->GetCamera()->SetBehindPlayer();	
}

//----------------------------------------------------

void ScrTogglePlayerControllable(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteControllable;
	bsData.Read(byteControllable);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->TogglePlayerControllable((int)byteControllable);
}

//----------------------------------------------------

void ScrPlaySound(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int iSound;
	float fX, fY, fZ;
	bsData.Read(iSound);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	pGame->PlaySoundFX(iSound, fX, fY, fZ);
}

//----------------------------------------------------

void ScrSetWorldBounds(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(pNetGame->m_WorldBounds[0]);
	bsData.Read(pNetGame->m_WorldBounds[1]);
	bsData.Read(pNetGame->m_WorldBounds[2]);
	bsData.Read(pNetGame->m_WorldBounds[3]);
}

//----------------------------------------------------

void ScrHaveSomeMoney(RPCParameters *rpcParams)
{
	int iAmount;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(iAmount);
	pGame->AddToLocalMoney(iAmount);
}

//----------------------------------------------------

void ScrSetPlayerFacingAngle(RPCParameters *rpcParams)
{
	float fAngle;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(fAngle);
	pGame->FindPlayerPed()->ForceTargetRotation(fAngle);
}

//----------------------------------------------------

void ScrResetMoney(RPCParameters *rpcParams)
{
	pGame->ResetLocalMoney();
}

//----------------------------------------------------

void ScrResetPlayerWeapons(RPCParameters *rpcParams)
{
	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->ClearAllWeapons();
	//pChatWindow->AddDebugMessage("Cleared weapons");
}

//----------------------------------------------------

void ScrGivePlayerWeapon(RPCParameters *rpcParams)
{
	int iWeaponID;
	int iAmmo;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(iWeaponID);
	bsData.Read(iAmmo);

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
	pPlayerPed->GiveWeapon(iWeaponID, iAmmo);
	//pChatWindow->AddDebugMessage("Gave weapon: %d with ammo %d", iWeaponID, iAmmo);
}

//----------------------------------------------------

void ScrRespawnVehicle(RPCParameters *rpcParams)
{
	VEHICLEID VehicleID;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(VehicleID);

	CPlayerPed *pPlayerPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();	
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	CVehicle *pVehicle = pVehiclePool->GetAt(VehicleID);

	if(pVehicle) {
		if( (pPlayerPed->m_pPed) &&
			(pPlayerPed->m_pPed->pVehicle == (DWORD)pVehicle->m_pVehicle) ) {
			MATRIX4X4 mat;
			pPlayerPed->GetMatrix(&mat);
			pPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X,mat.pos.Y,mat.pos.Z + 1.0f);	
		}

		pVehiclePool->SetForRespawn(VehicleID);
	}
		
}

//----------------------------------------------------

void ScrDeathMessage(RPCParameters *rpcParams)
{
	BYTE byteKiller, byteKillee, byteWeapon;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PCHAR szKillerName = NULL;
	PCHAR szKilleeName = NULL;
	DWORD dwKillerColor = 0;
	DWORD dwKilleeColor = 0;

	bsData.Read(byteKiller);
	bsData.Read(byteKillee);
	bsData.Read(byteWeapon);

	//pChatWindow->AddDebugMessage("RawDeath: %u %u %u",byteKiller,byteKillee,byteWeapon);

	if(byteKillee == INVALID_PLAYER_ID) return;

	// Determine the killer's name and color
	if(byteKiller == INVALID_PLAYER_ID) {
		szKillerName = NULL; dwKillerColor = 0;
	} else {
		if(pPlayerPool->GetLocalPlayerID() == byteKiller) {
			szKillerName = (PCHAR)pPlayerPool->GetLocalPlayer()->GetName();
			dwKillerColor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
		} else {
			CRemotePlayer* pPlayer = pPlayerPool->GetAt(byteKiller);
			if (pPlayer != NULL) {
				szKillerName = (PCHAR)pPlayer->GetName();
				dwKillerColor = pPlayer->GetPlayerColorAsARGB();
			} else {
				//pChatWindow->AddDebugMessage("Slot State Killer FALSE");
				szKillerName = NULL; dwKillerColor = 0;
			}
		}
	}

	// Determine the killee's name and color
	if(pPlayerPool->GetLocalPlayerID() == byteKillee) {
		szKilleeName = (PCHAR)pPlayerPool->GetLocalPlayer()->GetName();
		dwKilleeColor = pPlayerPool->GetLocalPlayer()->GetPlayerColorAsARGB();
	} else {
		CRemotePlayer* pPlayer = pPlayerPool->GetAt(byteKillee);
		if (pPlayer != NULL) {
			szKilleeName = (PCHAR)pPlayer->GetName();
			dwKilleeColor = pPlayer->GetPlayerColorAsARGB();
		} else {
			//pChatWindow->AddDebugMessage("Slot State Killee FALSE");
			szKilleeName = NULL; dwKilleeColor = 0;
		}
	}

	if(pDeathWindow) pDeathWindow->AddMessage(szKillerName,szKilleeName,dwKillerColor,dwKilleeColor,byteWeapon);
}

//----------------------------------------------------

void ScrSetMapIcon(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteIndex;
	BYTE byteIcon;
	DWORD byteColor;
	float fPos[3];
	unsigned char ucStyle;

	bsData.Read(byteIndex);
	bsData.Read(fPos[0]);
	bsData.Read(fPos[1]);
	bsData.Read(fPos[2]);
	bsData.Read(byteIcon);
	bsData.Read(byteColor);
	bsData.Read(ucStyle);

	pNetGame->SetMapIcon(byteIndex, fPos[0], fPos[1], fPos[2], byteIcon, byteColor, ucStyle);
}

void ScrDisableMapIcon(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteIndex;

	bsData.Read(byteIndex);

	pNetGame->DisableMapIcon(byteIndex);
}

void ScrSetPlayerArmour(RPCParameters *rpcParams)
{
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	float fArmour;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(fArmour);

	pLocalPlayer->GetPlayerPed()->SetArmour(fArmour);
}

void ScrSetWeaponAmmo(RPCParameters *rpcParams)
{
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	BYTE byteWeapon;
	WORD wordAmmo;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteWeapon);
	bsData.Read(wordAmmo);

	pLocalPlayer->GetPlayerPed()->SetAmmo(byteWeapon, wordAmmo);
}

//----------------------------------------------------

void ScrSetGravity(RPCParameters *rpcParams)
{
	float fGravity;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(fGravity);

	pGame->SetGravity(fGravity);
}

//----------------------------------------------------

void ScrSetVehicleHealth(RPCParameters *rpcParams)
{
	float fVehicleHealth;
	VEHICLEID VehicleID;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(VehicleID);
	bsData.Read(fVehicleHealth);

	if (pNetGame->GetVehiclePool()->GetSlotState(VehicleID))
	{
		pNetGame->GetVehiclePool()->GetAt(VehicleID)->SetHealth(fVehicleHealth);
	}
}

//----------------------------------------------------

void ScrAttachTrailerToVehicle(RPCParameters *rpcParams)
{
	VEHICLEID TrailerID, VehicleID;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(TrailerID);
	bsData.Read(VehicleID);
	CVehicle* pTrailer = pNetGame->GetVehiclePool()->GetAt(TrailerID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	
	pVehicle->SetTrailer(pTrailer);
	pVehicle->AttachTrailer();
}

//----------------------------------------------------

void ScrDetachTrailerFromVehicle(RPCParameters *rpcParams)
{
	VEHICLEID VehicleID;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(VehicleID);
	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);

	pVehicle->DetachTrailer();
	pVehicle->SetTrailer(NULL);
}

//----------------------------------------------------

void ScrCreateObject(RPCParameters *rpcParams)
{
	byte byteObjectID;
	int iModel;
	VECTOR vecPos, vecRot;
	float fDrawDist;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteObjectID);
	bsData.Read(iModel);

	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);

	bsData.Read(vecRot.X);
	bsData.Read(vecRot.Y);
	bsData.Read(vecRot.Z);

	bsData.Read(fDrawDist);

	CObjectPool* pObjectPool =	pNetGame->GetObjectPool();
	pObjectPool->New(byteObjectID, iModel, vecPos, vecRot, fDrawDist);
}

//----------------------------------------------------

void ScrSetObjectPos(RPCParameters *rpcParams)
{
	byte byteObjectID;
	float fX, fY, fZ, fRotation;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteObjectID);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);
	bsData.Read(fRotation);

	CObjectPool*	pObjectPool =	pNetGame->GetObjectPool();
	CObject*		pObject		=	pObjectPool->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->TeleportTo(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrSetObjectRotation(RPCParameters *rpcParams)
{
	byte byteObjectID;
	float fX, fY, fZ;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteObjectID);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fZ);

	CObjectPool*	pObjectPool =	pNetGame->GetObjectPool();
	CObject*		pObject		=	pObjectPool->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->InstantRotate(fX, fY, fZ);
	}
}

//----------------------------------------------------

void ScrDestroyObject(RPCParameters *rpcParams)
{
	byte byteObjectID;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteObjectID);

	CObjectPool* pObjectPool =	pNetGame->GetObjectPool();
	if (pObjectPool->GetAt(byteObjectID))
	{
		pObjectPool->Delete(byteObjectID);
	}
}

void ScrSetObjectScale(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteObjectID;
	CObject* pObject;
	float fScale;

	if (pNetGame->GetObjectPool() && in.GetNumberOfUnreadBits() == 40) {
		if (in.Read(byteObjectID) && in.Read(fScale)) {
			pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
			if (pObject != NULL) {
				pObject->SetScale(fScale);
			}
		}
	}
}

//----------------------------------------------------

void ScrSetPlayerVirtualWorld(RPCParameters *rpcParams)
{
	// Sets which players are visible to the local player and which aren't
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteCount = rpcParams->numberOfBitsOfData / 16;
	BYTE bytePlayer;
	int iVW;
	
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	PLAYERID byteLocal = pPlayerPool->GetLocalPlayerID();	
	
	for (BYTE i = 0; i < byteCount; i++)
	{
		bsData.Read(bytePlayer);
		bsData.Read(iVW);
		if (bytePlayer == byteLocal)
		{
			CLocalPlayer* pPlayer = pPlayerPool->GetLocalPlayer();
			if (pPlayer) pPlayer->SetVirtualWorld(iVW);
		}
		else
		{
			CRemotePlayer* pPlayer = pPlayerPool->GetAt(bytePlayer);
			if (pPlayer) pPlayer->SetVirtualWorld(iVW);
		}
	}
}

//----------------------------------------------------

void ScrSetVehicleVirtualWorld(RPCParameters *rpcParams)
{
	// Sets which players are visible to the local player and which aren't
	int iCount = rpcParams->numberOfBitsOfData / 24;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID Vehicle;
	int iVW;
	
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	
	for (int i = 0; i < iCount; i++)
	{
		bsData.Read(Vehicle);
		bsData.Read(iVW);
		pVehiclePool->SetVehicleVirtualWorld(Vehicle, iVW);
	}
}

void ScrCreateExplosion(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	float X, Y, Z, Radius;
	int   iType;

	bsData.Read(X);
	bsData.Read(Y);
	bsData.Read(Z);
	bsData.Read(iType);
	bsData.Read(Radius);

	ScriptCommand(&create_explosion_with_radius, X, Y, Z, iType, Radius);
}

void ScrShowNameTag(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE bytePlayerID;
	BYTE byteShow;

	bsData.Read(bytePlayerID);
	bsData.Read(byteShow);

	if (pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID))
	{
		pNetGame->GetPlayerPool()->GetAt(bytePlayerID)->ShowNameTag(byteShow);
	}
}

void ScrMoveObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteObjectID;
	float curx, cury, curz, newx, newy, newz, speed;

	bsData.Read(byteObjectID);
	bsData.Read(curx);
	bsData.Read(cury);
	bsData.Read(curz);
	bsData.Read(newx);
	bsData.Read(newy);
	bsData.Read(newz);
	bsData.Read(speed);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->TeleportTo(curx, cury, curz);
		pObject->MoveTo(newx, newy, newz, speed);
	}
}

void ScrStopObject(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	BYTE byteObjectID;
	float newx, newy, newz;

	bsData.Read(byteObjectID);
	bsData.Read(newx);
	bsData.Read(newy);
	bsData.Read(newz);

	CObject* pObject = pNetGame->GetObjectPool()->GetAt(byteObjectID);
	if (pObject)
	{
		pObject->MoveTo(newx, newy, newz, pObject->m_fMoveSpeed);
	}
}

void ScrNumberPlate(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	VEHICLEID Vehicle;
	CHAR cNumberPlate[9];
	
	bsData.Read(Vehicle);
	bsData.Read(cNumberPlate, 9);
	strcpy_s(pNetGame->GetVehiclePool()->m_charNumberPlate[Vehicle], cNumberPlate);
}

//----------------------------------------------------

void ScrTogglePlayerSpectating(RPCParameters *rpcParams)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	uint8_t bToggle;
	bsData.Read(bToggle);
	pPlayerPool->GetLocalPlayer()->ToggleSpectating(bToggle);
}

void ScrSetPlayerSpectating(RPCParameters *rpcParams)
{
	BYTE bytePlayerID;
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	bsData.Read(bytePlayerID);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool->GetSlotState(bytePlayerID)) {
		pPlayerPool->GetAt(bytePlayerID)->SetState(PLAYER_STATE_SPECTATING);
	}
}

#define SPECTATE_TYPE_NORMAL	1
#define SPECTATE_TYPE_FIXED		2
#define SPECTATE_TYPE_SIDE		3

void ScrPlayerSpectatePlayer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	BYTE bytePlayerID;
    BYTE byteMode;
	
	bsData.Read(bytePlayerID);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 4;
	}
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectatePlayer(bytePlayerID);
}

void ScrPlayerSpectateVehicle(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	//pChatWindow->AddDebugMessage("ScrPlayerSpectateVehicle");
	
	VEHICLEID VehicleID;
    BYTE byteMode;
	
	bsData.Read(VehicleID);
	bsData.Read(byteMode);

	switch (byteMode) {
		case SPECTATE_TYPE_FIXED:
			byteMode = 15;
			break;
		case SPECTATE_TYPE_SIDE:
			byteMode = 14;
			break;
		default:
			byteMode = 3;
	}
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
	pLocalPlayer->m_byteSpectateMode = byteMode;
	pLocalPlayer->SpectateVehicle(VehicleID);
}

void ScrRemoveComponent(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	VEHICLEID VehicleID;
    DWORD dwComponent;
	
	bsData.Read(VehicleID);
	bsData.Read(dwComponent);

	int iVehicleID;
	//int iComponent;

	if(!pNetGame) return;

	iVehicleID = pNetGame->GetVehiclePool()->FindGtaIDFromID(VehicleID);
	if(iVehicleID) ScriptCommand(&remove_component, iVehicleID, (int)dwComponent);
}

void ScrForceSpawnSelection(RPCParameters *rpcParams)
{
	pNetGame->GetPlayerPool()->GetLocalPlayer()->ReturnToClassSelection();
}

void ScrAttachObjectToPlayer(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteObjectID, bytePlayerID;
	float OffsetX, OffsetY, OffsetZ, rX, rY, rZ;

	bsData.Read( byteObjectID );
	bsData.Read( bytePlayerID );

	bsData.Read( OffsetX );
	bsData.Read( OffsetY );
	bsData.Read( OffsetZ );

	bsData.Read( rX );
	bsData.Read( rY );
	bsData.Read( rZ );

	try {

	CObject* pObject =	pNetGame->GetObjectPool()->GetAt(	byteObjectID );

	if ( !pObject )
		return;

	if ( bytePlayerID == pNetGame->GetPlayerPool()->GetLocalPlayerID() )
	{
		CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		ScriptCommand( &attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
			OffsetX,
			OffsetY,
			OffsetZ, 
			rX,
			rY,
			rZ);
	} else {
		CRemotePlayer* pPlayer =	pNetGame->GetPlayerPool()->GetAt(	bytePlayerID );

		if ( !pPlayer )
			return;

		ScriptCommand( &attach_object_to_actor, pObject->m_dwGTAId, pPlayer->GetPlayerPed()->m_dwGTAId,
																	OffsetX,
																	OffsetY,
																	OffsetZ, 
																	rX,
																	rY,
																	rZ);
	}

	} catch(...) {}
}

void ScrInitMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	bool bColumns; // 0 = 1, 1 = 2
	CHAR cText[MAX_MENU_LINE];
	float fX;
	float fY;
	float fCol1;
	float fCol2 = 0.0;
	MENU_INT MenuInteraction;
	
	bsData.Read(byteMenuID);
	bsData.Read(bColumns);
	bsData.Read(cText, MAX_MENU_LINE);
	bsData.Read(fX);
	bsData.Read(fY);
	bsData.Read(fCol1);
	if (bColumns) bsData.Read(fCol2);
	bsData.Read(MenuInteraction.bMenu);
	for (BYTE i = 0; i < MAX_MENU_ITEMS; i++)
	{
		bsData.Read(MenuInteraction.bRow[i]);
	}

	CMenu* pMenu;
	
	if (pMenuPool->GetSlotState(byteMenuID))
	{
		pMenuPool->Delete(byteMenuID);
	}
	
	pMenu = pMenuPool->New(byteMenuID, cText, fX, fY, ((BYTE)bColumns) + 1, fCol1, fCol2, &MenuInteraction);
	
	if (!pMenu) return;
	
	BYTE byteColCount;
	bsData.Read(cText, MAX_MENU_LINE);
	pMenu->SetColumnTitle(0, cText);
	
	bsData.Read(byteColCount);
	for (BYTE i = 0; i < byteColCount; i++)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->AddMenuItem(0, i, cText);
	}
	
	if (bColumns)
	{
		bsData.Read(cText, MAX_MENU_LINE);
		pMenu->SetColumnTitle(1, cText);
		
		bsData.Read(byteColCount);
		for (BYTE i = 0; i < byteColCount; i++)
		{
			bsData.Read(cText, MAX_MENU_LINE);
			pMenu->AddMenuItem(1, i, cText);
		}
	}
}

void ScrShowMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	pNetGame->GetMenuPool()->ShowMenu(byteMenuID);
}

void ScrHideMenu(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	if(!pNetGame) return;
	CMenuPool* pMenuPool = pNetGame->GetMenuPool();

	BYTE byteMenuID;
	bsData.Read(byteMenuID);
	pNetGame->GetMenuPool()->HideMenu(byteMenuID);
}

void ScrSetPlayerWantedLevel(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	if(!pGame) return;

	BYTE byteLevel;
	bsData.Read(byteLevel);
	pGame->SetWantedLevel(byteLevel);
}

void ScrShowTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		TEXT_DRAW_TRANSMIT TextDrawTransmit;
		CHAR cText[MAX_TEXT_DRAW_LINE];

		bsData.Read(wTextID);
		bsData.Read((PCHAR)&TextDrawTransmit, sizeof (TEXT_DRAW_TRANSMIT));
		bsData.Read(cText, MAX_TEXT_DRAW_LINE);
		pTextDrawPool->New(wTextID, &TextDrawTransmit, cText);
	}
}

void ScrHideTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		bsData.Read(wTextID);
		pTextDrawPool->Delete(wTextID);
	}
}

void ScrEditTextDraw(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
	if (pTextDrawPool)
	{
		WORD wTextID;
		CHAR cText[MAX_TEXT_DRAW_LINE];
		bsData.Read(wTextID);
		bsData.Read(cText, MAX_TEXT_DRAW_LINE);
		CTextDraw* pText = pTextDrawPool->GetAt(wTextID);
		if (pText) pText->SetText(cText);
	}
}

void ScrAddGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		float minx, miny, maxx, maxy;
		DWORD dwColor;
		bsData.Read(wZoneID);
		bsData.Read(minx);
		bsData.Read(miny);
		bsData.Read(maxx);
		bsData.Read(maxy);
		bsData.Read(dwColor);
//		pChatWindow->AddDebugMessage("called %d %f %f %f %f %x", wZoneID, minx, miny, maxx, maxy, dwColor);
		pGangZonePool->New(wZoneID, minx, miny, maxx, maxy, dwColor);
	}
}

void ScrRemoveGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->Delete(wZoneID);
	}
}

void ScrFlashGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		DWORD dwColor;
		bsData.Read(wZoneID);
		bsData.Read(dwColor);
		pGangZonePool->Flash(wZoneID, dwColor);
	}
}

void ScrStopFlashGangZone(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
	if (pGangZonePool)
	{
		WORD wZoneID;
		bsData.Read(wZoneID);
		pGangZonePool->StopFlash(wZoneID);
	}
}

//----------------------------------------------------

void ScrApplyAnimation(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE bytePlayerID;
	BYTE byteAnimLibLen;
	BYTE byteAnimNameLen;
	char szAnimLib[256];
	char szAnimName[256];
	float fS;
	bool opt1,opt2,opt3,opt4;
	int  opt5;
	CPlayerPool *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	memset(szAnimLib,0,256);
	memset(szAnimName,0,256);

	bsData.Read(bytePlayerID);
	bsData.Read(byteAnimLibLen);
	bsData.Read(szAnimLib,byteAnimLibLen);
	bsData.Read(byteAnimNameLen);
	bsData.Read(szAnimName,byteAnimNameLen);
	bsData.Read(fS);
	bsData.Read(opt1);
	bsData.Read(opt2);
	bsData.Read(opt3);
	bsData.Read(opt4);
	bsData.Read(opt5);

	szAnimLib[byteAnimLibLen] = '\0';
	szAnimName[byteAnimNameLen] = '\0';

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(bytePlayerID)) {
				pPlayerPed = pPlayerPool->GetAt(bytePlayerID)->GetPlayerPed();
			}
		}
		if(pPlayerPed) {
			try {
				pPlayerPed->ApplyAnimation(szAnimName,szAnimLib,fS,
					(int)opt1,(int)opt2,(int)opt3,(int)opt4,(int)opt5);
			} catch(...) {}
		}
	}
}

//----------------------------------------------------

void ScrClearAnimations(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE bytePlayerID;
	bsData.Read(bytePlayerID);
	MATRIX4X4 mat;

	CPlayerPool *pPlayerPool=NULL;
	CPlayerPed *pPlayerPed=NULL;

	pPlayerPool = pNetGame->GetPlayerPool();

	if(pPlayerPool) {
		// Get the CPlayerPed for this player
		if(bytePlayerID == pPlayerPool->GetLocalPlayerID()) {
			pPlayerPed = pPlayerPool->GetLocalPlayer()->GetPlayerPed();
		}
		else {
			if(pPlayerPool->GetSlotState(bytePlayerID)) {
				pPlayerPed = pPlayerPool->GetAt(bytePlayerID)->GetPlayerPed();
			}
		}
		if(pPlayerPed) {
			try {
				
				pPlayerPed->GetMatrix(&mat);
				pPlayerPed->TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);

			} catch(...) {}
		}
	}
}

//----------------------------------------------------

void ScrSetSpecialAction(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	BYTE byteSpecialAction;
	bsData.Read(byteSpecialAction);
	
	CPlayerPool *pPool=pNetGame->GetPlayerPool();
	if(pPool) pPool->GetLocalPlayer()->ApplySpecialAction(byteSpecialAction);
}

//----------------------------------------------------

void ScrEnableStuntBonus(RPCParameters *rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	bool bStuntBonusEnabled;
	bsData.Read(bStuntBonusEnabled);
	pGame->EnableStuntBonus(bStuntBonusEnabled);
}

static void ScrSetVehicle(RPCParameters* rpcParams)
{
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (pVehiclePool) {
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);
		if (in.GetNumberOfUnreadBits() >= 24) {
			unsigned char ucOP = 0;
			VEHICLEID nVehicleID = 0;

			in.Read(ucOP);
			in.Read(nVehicleID);

			CVehicle* pVehicle = pVehiclePool->GetAt(nVehicleID);
			if (pVehicle != NULL) {
				switch (ucOP) {
				case 1:
					pVehicle->Fix();
					break;
				case 2: {
					if (in.GetNumberOfUnreadBits() == 4) {
						in.ReadBits((unsigned char*)&pVehiclePool->m_Windows[nVehicleID], 4);

						// TODO: Need checking at and add model filtering here and/or server
						// Seems like it works on most of the vehicles, but on some vehicles it crashes the game,
						// with gta_sa.exe:0x6D30B5 crash address. ecx at [ecx+18h] looks like not initialized.
						if (pVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
							pVehicle->ToggleWindow(10, pVehiclePool->m_Windows[nVehicleID].bDriver);
							pVehicle->ToggleWindow(8, pVehiclePool->m_Windows[nVehicleID].bPassenger);
							pVehicle->ToggleWindow(11, pVehiclePool->m_Windows[nVehicleID].bBackLeft);
							pVehicle->ToggleWindow(9, pVehiclePool->m_Windows[nVehicleID].bBackRight);
						}
					}
					break;
				}
				case 3: {
					pVehicle->ToggleTaxiLight(in.ReadBit());
					break;
				}
				case 4: {
					pVehicle->ToggleEngine(in.ReadBit());
					break;
				}
				case 5: {
					pVehicle->SetLightState(in.ReadBit());
					break;
				}
				case 6: {
					pVehicle->SetFeature(in.ReadBit());
					break;
				}
				case 7: {
					pVehicle->SetVisibility(in.ReadBit());
					break;
				}
				case 8: {
					if (in.GetNumberOfUnreadBits() == 4) {
						in.ReadBits((unsigned char*)&pVehiclePool->m_Doors[nVehicleID], 4);

						pVehicle->ToggleDoor(2, 10, pVehiclePool->m_Doors[nVehicleID].bDriver ? 1.0f : 0.0f);
						pVehicle->ToggleDoor(3, 8, pVehiclePool->m_Doors[nVehicleID].bPassenger ? 1.0f : 0.0f);
						pVehicle->ToggleDoor(4, 11, pVehiclePool->m_Doors[nVehicleID].bBackLeft ? 1.0f : 0.0f);
						pVehicle->ToggleDoor(5, 9, pVehiclePool->m_Doors[nVehicleID].bBackRight ? 1.0f : 0.0f);
					}
					break;
				}
				case 9:
				{
					pVehicle->Explode();
					break;
				}
				}
			}
		}
	}
}

static void ScrSetPlayer(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

	int iOP = 0;
	in.Read(iOP);
	switch (iOP)
	{
	case 1:
	{
		int iDrunkLevel = 0;
		in.Read(iDrunkLevel);
		pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetDrunkLevel(iDrunkLevel);
		break;
	}
	case 2:
	{
		unsigned char ucSkill = 0;
		unsigned int uiLevel = 0;
		in.Read(ucSkill);
		in.Read(uiLevel);
		pGame->SetWeaponSkill(ucSkill, uiLevel);
		break;
	}
	case 3:
	{
		unsigned char ucWeaponId = 0;
		in.Read(ucWeaponId);
		pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetArmedWeapon(ucWeaponId);
		break;
	}
	case 4:
	{
		unsigned char style, move;
		in.Read(style);
		in.Read(move);
		pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->SetFightingStyle(style, move);
		break;
	}
	case 5:
	{
		float fVal = 0.0f;
		in.Read(fVal);
		pGame->SetMaxHealth(fVal);
		break;
	}
	case 6:
	{
		unsigned char ucLevel = 0;
		if (in.Read(ucLevel) && (0 <= ucLevel && ucLevel <= 100))
		{
			pGame->SetBlurLevel(ucLevel);
		}
	}
	}
}

static void ScrVehicleVelocity(RPCParameters* rpcParams)
{
	VEHICLEID nVehicleID;
	CVehicle* pVehicle;
	float fX, fY, fZ;

	if (rpcParams->numberOfBitsOfData == 113 && pNetGame->GetVehiclePool()) {
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

		in.Read(nVehicleID);

		pVehicle = pNetGame->GetVehiclePool()->GetAt(nVehicleID);
		if (pVehicle) {
			in.Read(fX);
			in.Read(fY);
			in.Read(fZ);

			if (in.ReadBit())
				pVehicle->SetTurnSpeedVector({ fX, fY, fZ });
			else
				pVehicle->SetMoveSpeedVector({ fX, fY, fZ });

			pVehicle->ApplyMoveSpeed();
		}
	}
}

static void ScrPlayerVelocity(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

	if (in.GetNumberOfUnreadBits() != 128)
		return;

	int iPlayerId = -1;
	float fX = 0.0f, fY = 0.0f, fZ = 0.0f;

	if (!in.Read(iPlayerId) && iPlayerId < 0)
		return;

	CPlayerPed* pPlayerPed = NULL;
	CPlayerPool* pPool = pNetGame->GetPlayerPool();
	if (pPool) {
		if (iPlayerId == pPool->GetLocalPlayerID())
			pPlayerPed = pPool->GetLocalPlayer()->GetPlayerPed();
		else
			if (pPool->GetSlotState(iPlayerId))
				pPlayerPed = pPool->GetAt(iPlayerId)->GetPlayerPed();

		if (pPlayerPed)
		{
			in.Read(fX);
			in.Read(fY);
			in.Read(fZ);

			pPlayerPed->SetMoveSpeedVector({ fX, fY, fZ });
			pPlayerPed->ApplyMoveSpeed();
		}
	}
}

static void ScrInterpolateCamera(RPCParameters* rpcParams)
{
	VECTOR vecFrom, vecTo;
	unsigned char ucMode;
	float fTime;

	if (rpcParams->numberOfBitsOfData == 233)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		vecFrom = 0.0f;
		vecTo = 0.0f;
		fTime = 0.0f;
		ucMode = 0;

		bsData.Read(vecFrom);
		bsData.Read(vecTo);
		bsData.Read(fTime);
		
		if (0.0f < fTime)
		{
			bsData.Read(ucMode);

			if (ucMode < 1 || ucMode > 2)
				ucMode = 2;

			if (pNetGame->GetPlayerPool())
				pNetGame->GetPlayerPool()->GetLocalPlayer()->m_bSpectateProcessed = true;

			if (bsData.ReadBit())
				pGame->GetCamera()->InterpolateCameraPos(&vecFrom, &vecTo, fTime, ucMode);
			else
				pGame->GetCamera()->InterpolateCameraLookAt(&vecFrom, &vecTo, fTime, ucMode);
		}
	}
}

static void ScrVehicleComponent(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID nVehicleId = 0;
	CVehicle* pVehicle;
	CVehiclePool* pPool = pNetGame->GetVehiclePool();
	if (in.Read(nVehicleId) && pPool && (pVehicle = pPool->GetAt(nVehicleId)) != NULL)
	{
		unsigned char ucComponent = 0;
		if (in.Read(ucComponent))
			if (in.ReadBit())
				pVehicle->ToggleComponent(ucComponent, 1.0f);
			else
				pVehicle->ToggleComponent(ucComponent, 0.0f);
	}
}

static void ScrSetGameSpeed(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream in((unsigned char*)Data, (iBitLength / 8) + 1, false);

	float fSpeed = 1.0f;
	if (in.Read(fSpeed))
	{
		pGame->SetGameSpeed(fSpeed);
	}
}

static void ScrToggleChatbox(RPCParameters* rpcParams)
{
	if (!pChatWindow) return;

	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	if (bsData.GetNumberOfUnreadBits() == 1)
	{
		pChatWindow->ForceHide(bsData.ReadBit());
	}
}

static void ScrToggleWidescreen(RPCParameters* rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	if (bsData.GetNumberOfUnreadBits() == 1) {
		pGame->GetCamera()->ToggleWidescreen(bsData.ReadBit());
	}
}

static void ScrSetScore(RPCParameters * rpcParams)
{
	PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	if (bsData.GetNumberOfUnreadBits() == 48) {
		unsigned short usPlayerId;
		int iScore;
		if (bsData.Read(usPlayerId) && usPlayerId < MAX_PLAYERS) {
			bsData.Read(iScore);
			pNetGame->GetPlayerPool()->UpdateScore(usPlayerId, iScore);
		}
	}
}

static void ScrSetShopName(RPCParameters* pParams)
{
	if (pParams->numberOfBitsOfData == 256)
	{
		char szSectionName[32];
		PCHAR Data = reinterpret_cast<PCHAR>(pParams->input);
		int iBitLength = pParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(szSectionName);

#ifdef _DEBUG
		if (pChatWindow)
			pChatWindow->AddDebugMessage("[DEBUG] Disabling interior script: %s", szSectionName);
#endif

		CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
		if (pPlayerPed) {
			pPlayerPed->SetEntryExit(szSectionName);
		}
	}
}

static void ScrAudioStream(RPCParameters* rpcParams)
{
	if (pAudioStream == NULL)
		return;

	unsigned short usUrlLen = 0, uiBitSize;
	float fX, fY, fZ, fDist;
	char* szUrl;

	if (16 <= rpcParams->numberOfBitsOfData)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usUrlLen);

		uiBitSize = (usUrlLen * 8) + 1;
		if (uiBitSize > 16385 ||
			(bsData.GetNumberOfUnreadBits() != uiBitSize &&
			bsData.GetNumberOfUnreadBits() != uiBitSize + 128))
		{
#ifdef _DEBUG
			pChatWindow->AddDebugMessage("Error: Malformed audio stream packet size.");
#endif
			return;
		}

		szUrl = (char*)malloc(usUrlLen + 1);
		if (szUrl == NULL)
		{
#ifdef _DEBUG
			pChatWindow->AddDebugMessage("Error: Allocation failed for audio stream url.");
#endif
			return;
		}

		bsData.Read(szUrl, usUrlLen);
		szUrl[usUrlLen] = 0;

		fX = fY = fZ = fDist = -1.0f;

		if (bsData.ReadBit() && bsData.GetNumberOfUnreadBits() == 128)
		{
			bsData.Read(fX);
			bsData.Read(fY);
			bsData.Read(fZ);

			bsData.Read(fDist);
		}

		pAudioStream->Play(szUrl, fX, fY, fZ, fDist);

		//free(szUrl); // Freeing up when the url passed to the audio thread
	}
	else
		pAudioStream->Stop();
}

static void ScrPlayCrimeReport(RPCParameters* rpcParams)
{
	//PLAYERID SuspectPlayerID;
	int iCrimeID;
	unsigned char ucVehicleModel, ucVehicleColor;
	VECTOR vecPosition;

	if (rpcParams->numberOfBitsOfData == 145 ||
		rpcParams->numberOfBitsOfData == 129)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		//bsData.Read(SuspectPlayerID);

		ucVehicleModel = ucVehicleColor = 255;

		if (bsData.ReadBit())
		{
			bsData.Read(ucVehicleModel);
			bsData.Read(ucVehicleColor);
		}

		bsData.Read(iCrimeID);
		bsData.Read(vecPosition.X);
		bsData.Read(vecPosition.Y);
		bsData.Read(vecPosition.Z);

		CGame::PlayCrimeReport(iCrimeID, &vecPosition, ucVehicleModel + 400, ucVehicleColor);
	}
}

//----------------------------------------------------

static void ScrSetActorPos(RPCParameters* rpcParams)
{
	unsigned short usActorID;
	CActor* pActor;
	VECTOR vecPos;

	if (pNetGame->GetActorPool() &&
		rpcParams->numberOfBitsOfData == 112)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usActorID);

		pActor = pNetGame->GetActorPool()->GetAt(usActorID);
		if (pActor)
		{
			bsData.Read(vecPos);

			pActor->TeleportTo(vecPos.X, vecPos.Y, vecPos.Z);
		}
	}
}

//----------------------------------------------------

static void ScrSetActorFacingAngle(RPCParameters* rpcParams)
{
	unsigned short usActorID;
	CActor* pActor;
	float fFacingAngle;

	if (pNetGame->GetActorPool() &&
		rpcParams->numberOfBitsOfData == 48)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usActorID);

		pActor = pNetGame->GetActorPool()->GetAt(usActorID);
		if (pActor)
		{
			bsData.Read(fFacingAngle);

			pActor->SetAngle(fFacingAngle);
		}
	}
}

//----------------------------------------------------

static void ScrSetActorHealth(RPCParameters* rpcParams)
{
	unsigned short usActorID;
	CActor* pActor;
	float fHealth;

	if (pNetGame->GetActorPool() &&
		rpcParams->numberOfBitsOfData == 48)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usActorID);

		pActor = pNetGame->GetActorPool()->GetAt(usActorID);
		if (pActor)
		{
			bsData.Read(fHealth);

			pActor->SetHealth(fHealth);
		}
	}
}

//----------------------------------------------------

static void ScrApplyActorAnimation(RPCParameters* rpcParams)
{
	unsigned short usActorID;
	CActor* pActor;
	unsigned char ucAnimLibLen, ucAnimNameLen;
	char szAnimLib[64] = { 0 }, szAnimName[64] = { 0 };
	float fDelta;
	bool bLoop, bLockX, bLockY, bFreeze;
	int iTime;

	if (pNetGame->GetActorPool() &&
		(rpcParams->numberOfBitsOfData >= 116 && rpcParams->numberOfBitsOfData <= 1124))
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usActorID);

		pActor = pNetGame->GetActorPool()->GetAt(usActorID);
		if (pActor)
		{
			//memset(szAnimLib, 0, sizeof(szAnimLib));
			//memset(szAnimName, 0, sizeof(szAnimName));

			bsData.Read(ucAnimLibLen);
			bsData.Read(szAnimLib, ucAnimLibLen);
			bsData.Read(ucAnimNameLen);
			bsData.Read(szAnimName, ucAnimNameLen);
			bsData.Read(fDelta);
			bsData.Read(bLoop);
			bsData.Read(bLockX);
			bsData.Read(bLockY);
			bsData.Read(bFreeze);
			bsData.Read(iTime);

			pActor->ApplyAnimation(szAnimName, szAnimLib, fDelta,
				bLoop, bLockX, bLockY, bFreeze, iTime);
		}
	}
}

//----------------------------------------------------

static void ScrClearActorAnimation(RPCParameters* rpcParams)
{
	unsigned short usActorID;
	CActor* pActor;

	if (pNetGame->GetActorPool() &&
		rpcParams->numberOfBitsOfData == 16)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		bsData.Read(usActorID);

		pActor = pNetGame->GetActorPool()->GetAt(usActorID);
		if (pActor)
		{
			pActor->ClearAnimations();
		}
	}
}

//----------------------------------------------------

static void ScrDisableVehicleCollision(RPCParameters* rpcParams)
{
	if (rpcParams->numberOfBitsOfData == 1)
	{
		PCHAR Data = reinterpret_cast<PCHAR>(rpcParams->input);
		int iBitLength = rpcParams->numberOfBitsOfData;

		RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

		pNetGame->m_bDisableVehicleCollision = bsData.ReadBit();
	}
}

static void ScrSetPlayerInterior(RPCParameters* rpcParams) {
	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;


	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);
	uint8_t byteInterior;
	bsData.Read(byteInterior);

	pGame->FindPlayerPed()->SetInterior(byteInterior);
}

//----------------------------------------------------

void RegisterScriptRPCs(RakClientInterface* pRakClient)
{
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDisplayGameText, ScrDisplayGameText);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetGravity, ScrSetGravity);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrForceSpawnSelection, ScrForceSpawnSelection);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPos, ScrSetPlayerPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraPos, ScrSetCameraPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraLookAt, ScrSetCameraLookAt);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerFacingAngle, ScrSetPlayerFacingAngle);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetFightingStyle, ScrSetFightingStyle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSkin, ScrSetPlayerSkin);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrApplyPlayerAnimation, ScrApplyPlayerAnimation);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrClearPlayerAnimations, ScrClearPlayerAnimations);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetSpawnInfo, ScrSetSpawnInfo);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrCreateExplosion, ScrCreateExplosion);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerHealth, ScrSetPlayerHealth);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerArmour, ScrSetPlayerArmour);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerColor, ScrSetPlayerColor);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerName, ScrSetPlayerName);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerPosFindZ, ScrSetPlayerPosFindZ);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetInterior, ScrSetPlayerInterior);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetMapIcon, ScrSetMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDisableMapIcon, ScrDisableMapIcon);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetCameraBehindPlayer, ScrSetCameraBehindPlayer);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetSpecialAction, ScrSetPlayerSpecialAction);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerSpectating, ScrTogglePlayerSpectating);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerSpectating, ScrSetPlayerSpectating);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectatePlayer, ScrPlayerSpectatePlayer);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlayerSpectateVehicle, ScrPlayerSpectateVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPutPlayerInVehicle, ScrPutPlayerInVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrVehicleParams, ScrVehicleParams);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrVehicleParamsEx, ScrVehicleParamsEx);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrHaveSomeMoney, ScrHaveSomeMoney);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrResetMoney, ScrResetMoney);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrLinkVehicle, ScrLinkVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrRemovePlayerFromVehicle, ScrRemovePlayerFromVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleHealth, ScrSetVehicleHealth);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehiclePos, ScrSetVehiclePos);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleVelocity, ScrSetVehicleVelocity);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrNumberPlate, ScrNumberPlate);
	//pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrInterpolateCamera, ScrInterpolateCamera);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrAddGangZone, ScrAddGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrRemoveGangZone, ScrRemoveGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrFlashGangZone, ScrFlashGangZone);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrStopFlashGangZone, ScrStopFlashGangZone);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrCreateObject, ScrCreateObject);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetObjectPos, ScrSetObjectPos);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrDestroyObject, ScrDestroyObject);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrPlaySound, ScrPlaySound);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetPlayerWantedLevel, ScrSetPlayerWantedLevel);

	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrTogglePlayerControllable, ScrTogglePlayerControllable);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrAttachObjectToPlayer, ScrAttachObjectToPlayer);
}

//----------------------------------------------------

/*void UnRegisterScriptRPCs(RakClientInterface* pRakClient)
{
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetSpawnInfo); // 32
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerTeam);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerName);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSkin);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerPosFindZ);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerHealth);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPutPlayerInVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemovePlayerFromVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerColor);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDisplayGameText);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetInterior);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraLookAt);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehiclePos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleZAngle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrVehicleParams);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetCameraBehindPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerControllable); // 50
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlaySound);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetWorldBounds);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHaveSomeMoney);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerFacingAngle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrResetMoney);
	UNREGISTER_STATIC_RPC(pRakClient, ScrResetPlayerWeapons);
	UNREGISTER_STATIC_RPC(pRakClient, ScrGivePlayerWeapon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRespawnVehicle); // 58
	UNREGISTER_STATIC_RPC(pRakClient, ScrLinkVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDeathMessage);
	//UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerMarker);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetMapIcon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDisableMapIcon);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetWeaponAmmo);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetGravity);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleHealth);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAttachTrailerToVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDetachTrailerFromVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrCreateObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetObjectPos);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetObjectRotation);
	UNREGISTER_STATIC_RPC(pRakClient, ScrDestroyObject); // Possible problem source
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerVirtualWorld);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicleVirtualWorld);
	UNREGISTER_STATIC_RPC(pRakClient, ScrCreateExplosion);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowNameTag);
	UNREGISTER_STATIC_RPC(pRakClient, ScrMoveObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrStopObject);
	UNREGISTER_STATIC_RPC(pRakClient, ScrNumberPlate);
	UNREGISTER_STATIC_RPC(pRakClient, ScrTogglePlayerSpectating);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerSpectating);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectatePlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrPlayerSpectateVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemoveComponent);
	UNREGISTER_STATIC_RPC(pRakClient, ScrForceSpawnSelection);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAttachObjectToPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrInitMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHideMenu);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayerWantedLevel);
	UNREGISTER_STATIC_RPC(pRakClient, ScrShowTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrHideTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrEditTextDraw);
	UNREGISTER_STATIC_RPC(pRakClient, ScrAddGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrRemoveGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrFlashGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrStopFlashGangZone);
	UNREGISTER_STATIC_RPC(pRakClient, ScrApplyAnimation);
	UNREGISTER_STATIC_RPC(pRakClient, ScrClearAnimations);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetSpecialAction);
	UNREGISTER_STATIC_RPC(pRakClient, ScrEnableStuntBonus);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetVehicle);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetPlayer);
	UNREGISTER_STATIC_RPC(pRakClient, ScrVehicleVelocity);
	UNREGISTER_STATIC_RPC(pRakClient, ScrInterpolateCamera);
	UNREGISTER_STATIC_RPC(pRakClient, ScrVehicleComponent);
	UNREGISTER_STATIC_RPC(pRakClient, ScrSetGameSpeed);
}*/

//----------------------------------------------------
