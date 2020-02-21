#include "knifechanger.h"
cskinchanger skinchanger;
RecvVarProxyFn oRecvnModelIndex;
RecvVarProxyFn fnSequenceProxyFn = NULL;
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
void SetViewModelSequence(const CRecvProxyData* pDataConst, void* pStruct, void* pOut) {
	// Make the incoming data editable.
	CRecvProxyData* pData = const_cast<CRecvProxyData*>(pDataConst);

	// Confirm that we are replacing our view model and not someone elses.
	C_BaseViewModel* pViewModel = (C_BaseViewModel*)pStruct;

	if (pViewModel) {
		IClientEntity* pOwner = g_pEntityList->GetClientEntityFromHandle(CBaseHandle(pViewModel->GetOwner()));

		// Compare the owner entity of this view model to the local player entity.
		if (pOwner && pOwner->EntIndex() == g_pEngine->GetLocalPlayer()) {
			// Get the filename of the current view model.
			void* pModel = g_pModelInfo->GetModel(pViewModel->GetModelIndex());

			const char* szModel = g_pModelInfo->GetModelName((model_t*)pModel);

			// Store the current sequence.
			int m_nSequence = pData->m_Value.m_Int;

			if (!strcmp(szModel, "models/weapons/v_knife_butterfly.mdl")) {
				// Fix animations for the Butterfly Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03); break;
				default:
					m_nSequence++;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_falchion_advanced.mdl")) {
				// Fix animations for the Falchion Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02); break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence--;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_push.mdl")) {
				// Fix animations for the Shadow Daggers.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
				case SEQUENCE_DEFAULT_LIGHT_MISS1:
				case SEQUENCE_DEFAULT_LIGHT_MISS2:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5); break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1); break;
				case SEQUENCE_DEFAULT_HEAVY_HIT1:
				case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence += 3; break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence += 2;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_survival_bowie.mdl")) {
				// Fix animations for the Bowie Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
				default:
					m_nSequence--;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_ursus.mdl")||!strcmp(szModel, "models/weapons/v_knife_skeleton.mdl")
					||!strcmp(szModel, "models/weapons/v_knife_outdoor.mdl")||!strcmp(szModel, "models/weapons/v_knife_canis.mdl")
					||!strcmp(szModel, "models/weapons/v_knife_cord.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
				m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
				break;
				case SEQUENCE_DEFAULT_LOOKAT01:
				m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
				break;
				default:
				m_nSequence++;
				break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_stiletto.mdl")) {
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(12, 13);
					break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_css.mdl.mdl"))
			{
				switch (m_nSequence)
				{
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomInt(14, 15);
						break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_widowmaker.mdl")) {
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(14, 15);
					break;
				}
			}
			// Set the fixed sequence.
			pData->m_Value.m_Int = m_nSequence;
		}
	}

	// Call original function with the modified data.
	fnSequenceProxyFn(pData, pStruct, pOut);
}

const char* cskinchanger::getKnifeModel(bool viewmodel = true)
{
	int gay = 9;
	//

	if (!g::pLocalEntity)
		return "models/weapons/v_knife_default_t.mdl";


	switch (g_Menu.Config.knife_model)
	{
	case 0:
		return viewmodel ? g::pLocalEntity->GetTeam() == 2 ? "models/weapons/v_knife_default_t.mdl" : "models/weapons/v_knife_default_ct.mdl" : g::pLocalEntity->GetTeam() == 2 ? "models/weapons/w_knife_default_t.mdl" : "models/weapons/w_knife_default_ct.mdl";
		break;

	case 1:
		return viewmodel ? "models/weapons/v_knife_m9_bay.mdl" : "models/weapons/w_knife_m9_bay.mdl";
		break;

	case 2:
		return viewmodel ? "models/weapons/v_knife_bayonet.mdl" : "models/weapons/w_knife_bayonet.mdl";
		break;

	case 3:
		return viewmodel ? "models/weapons/v_knife_flip.mdl" : "models/weapons/w_knife_flip.mdl";
		break;

	case 4:
		return viewmodel ? "models/weapons/v_knife_gut.mdl" : "models/weapons/w_knife_gut.mdl";
		break;

	case 5:
		return viewmodel ? "models/weapons/v_knife_karam.mdl" : "models/weapons/w_knife_karam.mdl";
		break;

	case 6:
		return viewmodel ? "models/weapons/v_knife_tactical.mdl" : "models/weapons/w_knife_tactical.mdl";
		break;

	case 7:
		return viewmodel ? "models/weapons/v_knife_falchion_advanced.mdl" : "models/weapons/w_knife_falchion_advanced.mdl";
		break;

	case 8:
		return viewmodel ? "models/weapons/v_knife_survival_bowie.mdl" : "models/weapons/w_knife_survival_bowie.mdl";
		break;

	case 9:
		return viewmodel ? "models/weapons/v_knife_butterfly.mdl" : "models/weapons/w_knife_butterfly.mdl";
		break;

	case 10:
		return viewmodel ? "models/weapons/v_knife_push.mdl" : "models/weapons/w_knife_push.mdl";
		break;

	case 11:
		return viewmodel ? "models/weapons/v_knife_gypsy_jackknife.mdl" : "models/weapons/w_knife_gypsy_jackknife.mdl";
		break;

	case 12:
		return viewmodel ? "models/weapons/v_knife_stiletto.mdl" : "models/weapons/w_knife_stiletto.mdl";
		break;

	case 13:
		return viewmodel ? "models/weapons/v_knife_ursus.mdl" : "models/weapons/w_knife_ursus.mdl";
		break;

	case 14:
		return viewmodel ? "models/weapons/v_knife_widowmaker.mdl" : "models/weapons/2_knife_widowmaker.mdl";
		break;

	case 15:
		return viewmodel ? "models/weapons/v_knife_css.mdl" : "models/weapons/w_knife_css.mdl";
		break;

	case 16:
		return viewmodel ? "models/weapons/v_knife_cord.mdl" : "models/weapons/w_knife_cord.mdl";
		break;

	case 17:
		return viewmodel ? "models/weapons/v_knife_canis.mdl" : "models/weapons/v_knife_canis.mdl";
		break;

	case 18:
		return viewmodel ? "models/weapons/v_knife_outdoor.mdl" : "models/weapons/w_knife_outdoor.mdl";
		break;

	case 19:
		return viewmodel ? "models/weapons/v_knife_skeleton.mdl" : "models/weapons/w_knife_skeleton.mdl";
		break;

	default:
		return "";
		break;
	}
}

int cskinchanger::getKnifeItemDefinitionIndex()
{
	int gay = 9;
	//

	if (!g::pLocalEntity)
		return 59;


	switch (g_Menu.Config.knife_model)
	{
	case 0:
		return g::pLocalEntity->GetTeam() == 1 ? 42 : 26;
		break;

	case 1:
		return 508;//m9bayonet
		break;

	case 2:
		return 500;//bayonet
		break;

	case 3:
		return 505;//flip
		break;

	case 4:
		return 506;//gut
		break;

	case 5:
		return 507;//karambit
		break;

	case 6:
		return 509; // Huntsman
		break;

	case 7:
		return 512;//falchion
		break;

	case 8:
		return 514;//bowie
		break;

	case 9:
		return 515;//butterfly
		break;

	case 10:
		return 516;//push
		break;

	case 11:
		return 520;//jacknife
		break;

	case 12:
		return 522;//stiletto
		break;

	case 13:
		return 519;//ursus
		break;

	case 14:
		return 523;//tallon
		break;

	case 15:
		return 503;//classic
		break;

	case 16:
		return 517;//cord
		break;

	case 17:
		return 518;//canis
		break;

	case 18:
		return 521;//outdoor
		break;

	default:
		return 525;//skeleton
		break;
	}
}
void Hooked_RecvProxy_Viewmodel(CRecvProxyData* pData, void* pStruct, void* pOut)
{
	// Get the knife view model id's
	int default_t = g_pModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	int default_ct = g_pModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	int iBayonet = g_pModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	int iButterfly = g_pModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	int iFlip = g_pModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
	int iGut = g_pModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
	int iKarambit = g_pModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
	int iM9Bayonet = g_pModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	int iHuntsman = g_pModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	int iFalchion = g_pModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	int iDagger = g_pModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
	int iBowie = g_pModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	int iGunGame = g_pModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");
	int iClasic = g_pModelInfo->GetModelIndex("models/weapons/v_knife_css.mdl");
	int iCord = g_pModelInfo->GetModelIndex("models/weapons/v_knife_cord.mdl");
	int iCanis = g_pModelInfo->GetModelIndex("models/weapons/v_knife_canis.mdl");
	int iOutdoor = g_pModelInfo->GetModelIndex("models/weapons/v_knife_outdoor.mdl");
	int iSkeleton = g_pModelInfo->GetModelIndex("models/weapons/v_knife_skeleton.mdl");

	// Get local player (just to stop replacing spectators knifes)
	auto pLocal = g::pLocalEntity;
	if (pLocal && g_Menu.Config.knife_model > 0)
	{
		// If we are alive and holding a default knife(if we already have a knife don't worry about changing)
		if (pLocal->IsAlive() && (
			pData->m_Value.m_Int == default_t ||
			pData->m_Value.m_Int == default_ct ||
			pData->m_Value.m_Int == iBayonet ||
			pData->m_Value.m_Int == iButterfly ||
			pData->m_Value.m_Int == iFlip ||
			pData->m_Value.m_Int == iGunGame ||
			pData->m_Value.m_Int == iClasic ||
			pData->m_Value.m_Int == iCord ||
			pData->m_Value.m_Int == iOutdoor ||
			pData->m_Value.m_Int == iSkeleton ||
			pData->m_Value.m_Int == iCanis ||
			pData->m_Value.m_Int == iGut ||
			pData->m_Value.m_Int == iKarambit ||
			pData->m_Value.m_Int == iM9Bayonet ||
			pData->m_Value.m_Int == iHuntsman ||
			pData->m_Value.m_Int == iFalchion ||
			pData->m_Value.m_Int == iDagger ||
			pData->m_Value.m_Int == iBowie))
		{
			pData->m_Value.m_Int = g_pModelInfo->GetModelIndex(skinchanger.getKnifeModel());
		}
	}

	// Carry on the to original proxy
	oRecvnModelIndex(pData, pStruct, pOut);
}
void cskinchanger::ApplyAAAHooks()
{
	ClientClass* pClass = g_pClientDll->GetAllClasses();
	while (pClass)
	{
		const char* pszName = pClass->pRecvTable->pNetTableName;

		if (!strcmp(pszName, "DT_BaseViewModel")) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->pRecvTable;

			for (int nIndex = 0; nIndex < pClass->pRecvTable->nProps; nIndex++) {
				RecvProp* pProp = &(pClass->pRecvTable->pProps[nIndex]);

				if (!pProp || strcmp(pProp->pVarName, "m_nSequence"))
					continue;

				// Store the original proxy function.
				fnSequenceProxyFn = (RecvVarProxyFn)pProp->ProxyFn;

				// Replace the proxy function with our sequence changer.
				pProp->ProxyFn = SetViewModelSequence;
			}
		}

		if (!strcmp(pszName, "DT_BaseViewModel"))
		{
			for (int i = 0; i < pClass->pRecvTable->nProps; i++)
			{
				RecvProp* pProp = &(pClass->pRecvTable->pProps[i]);
				const char* name = pProp->pVarName;

				// Knives
				if (!strcmp(name, "m_nModelIndex"))
				{
					oRecvnModelIndex = (RecvVarProxyFn)pProp->ProxyFn;
					pProp->ProxyFn = (RecvVarProxyFn)Hooked_RecvProxy_Viewmodel;
				}
			}
		}
		pClass = pClass->pNext;
	}
}