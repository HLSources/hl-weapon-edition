#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "weapons.h"
#include "decals.h"
#include "soundent.h"

class CFuncMortarField : public CBaseToggle
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );

	// Bmodels don't go across transitions
	virtual int	ObjectCaps( void ) { return CBaseToggle :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	void EXPORT FieldUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int m_iszXController;
	int m_iszYController;
	float m_flSpread;
	float m_flDelay;
	int m_iCount;
	int m_fControl;
};

LINK_ENTITY_TO_CLASS( func_mortar_field, CFuncMortarField );

TYPEDESCRIPTION	CFuncMortarField::m_SaveData[] = 
{
	DEFINE_FIELD( CFuncMortarField, m_iszXController, FIELD_STRING ),
	DEFINE_FIELD( CFuncMortarField, m_iszYController, FIELD_STRING ),
	DEFINE_FIELD( CFuncMortarField, m_flSpread, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncMortarField, m_flDelay, FIELD_FLOAT ),
	DEFINE_FIELD( CFuncMortarField, m_iCount, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncMortarField, m_fControl, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CFuncMortarField, CBaseToggle );


void CFuncMortarField :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iszXController"))
	{
		m_iszXController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszYController"))
	{
		m_iszYController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flSpread"))
	{
		m_flSpread = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fControl"))
	{
		m_fControl = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iCount"))
	{
		m_iCount = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}


// Drop bombs from above
void CFuncMortarField :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_NONE;
	SetBits( pev->effects, EF_NODRAW );
	SetUse( FieldUse );
	Precache();
}

void CFuncMortarField :: Precache( void )
{
	PRECACHE_SOUND ("weapons/mortar.wav");
	PRECACHE_SOUND ("weapons/mortarhit.wav");
}

// If connected to a table, then use the table controllers, else hit where the trigger is.
void CFuncMortarField :: FieldUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	Vector vecStart;

	vecStart.x = RANDOM_FLOAT( pev->mins.x, pev->maxs.x );
	vecStart.y = RANDOM_FLOAT( pev->mins.y, pev->maxs.y );
	vecStart.z = pev->maxs.z;

	switch( m_fControl )
	{
	case 0:	// random
		break;
	case 1: // Trigger Activator
		if (pActivator != NULL)
		{
			vecStart.x = pActivator->pev->origin.x;
			vecStart.y = pActivator->pev->origin.y;
		}
		break;
	case 2: // table
		{
			CBaseEntity *pController;

			if (!FStringNull(m_iszXController))
			{
				pController = UTIL_FindEntityByTargetname( NULL, STRING(m_iszXController));
				if (pController != NULL)
				{
					vecStart.x = pev->mins.x + pController->pev->ideal_yaw * (pev->size.x);
				}
			}
			if (!FStringNull(m_iszYController))
			{
				pController = UTIL_FindEntityByTargetname( NULL, STRING(m_iszYController));
				if (pController != NULL)
				{
					vecStart.y = pev->mins.y + pController->pev->ideal_yaw * (pev->size.y);
				}
			}
		}
		break;
	}

	int pitch = RANDOM_LONG(95,124);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/mortar.wav", 1.0, ATTN_NONE, 0, pitch);	

	float t = 2.5;
	for (int i = 0; i < m_iCount; i++)
	{
		Vector vecSpot = vecStart;
		vecSpot.x += RANDOM_FLOAT( -m_flSpread, m_flSpread );
		vecSpot.y += RANDOM_FLOAT( -m_flSpread, m_flSpread );

		TraceResult tr;
		UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -1 ) * 4096, ignore_monsters, ENT(pev), &tr );

		edict_t *pentOwner = NULL;
		if (pActivator)	pentOwner = pActivator->edict();

		CBaseEntity *pMortar = Create("mortar", tr.vecEndPos, Vector( 0, 0, 0 ), pentOwner );
		pMortar->pev->nextthink = gpGlobals->time + t;
		t += RANDOM_FLOAT( 0.3, 0.8 );
	}
}


class CMortar : public CBaseEntity
{
public:
	void Spawn( void );
	void EXPORT MortarExplode( void );
};
LINK_ENTITY_TO_CLASS( mortar, CMortar );

void CMortar::Spawn( )
{
	pev->movetype	= MOVETYPE_NONE;
	pev->solid	= SOLID_NOT;
	SetThink( MortarExplode );
}

void CMortar::MortarExplode( void )
{
	TraceResult tr;
	Vector vecSpot = pev->origin + Vector(0,0,1024);
	Vector vecEnd = pev->origin - Vector(0,0,1024);
	UTIL_TraceLine( vecSpot, vecEnd, dont_ignore_monsters, ENT(pev), &tr );

	entvars_t *pevOwner = VARS( pev->owner );
	::RadiusDamage( pev->origin, pev, pevOwner, 400, 800, CLASS_NONE, DMG_BLAST);
	UTIL_ScreenShake( tr.vecEndPos, 25.0, 150.0, 1.0, pev->dmg*3 );
	FX_Explosion( tr.vecEndPos, EXPLOSION_MORTAR );

	int tex = (int)TEXTURETYPE_Trace(&tr, vecSpot, vecEnd);
	CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
	FX_ImpRocket( tr.vecEndPos, tr.vecPlaneNormal, pEntity->IsBSPModel()?1:0, BULLET_NORMEXP, (float)tex );
	UTIL_Remove( this );
}