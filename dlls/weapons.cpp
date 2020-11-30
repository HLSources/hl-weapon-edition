#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"

extern CGraph	WorldGraph;
#define NOT_USED 255

DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL  	const char *g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short	g_sModelIndexSmoke;// smoke beam

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

extern int gmsgCurWeapon;
MULTIDAMAGE gMultiDamage;

//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a 
// player can carry.
//=========================================================
int MaxAmmoCarry( int iszName )
{
	for ( int i = 0;  i < MAX_WEAPONS; i++ )
	{
		if ( CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp( STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1 ) )
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		if ( CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp( STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2 ) )
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT( at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING( iszName ) );
	return -1;
}

	
/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount	= 0;
	gMultiDamage.type = 0;
}


void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker )
{
	Vector		vecSpot1;//where blood comes from
	Vector		vecDir;//direction blood should go
	TraceResult	tr;
	
	if ( !gMultiDamage.pEntity )
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type );
}


// GLOBALS USED:
//		gMultiDamage

void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if ( !pEntity )
		return;
	
	gMultiDamage.type |= bitsDamageType;

	if ( pEntity != gMultiDamage.pEntity )
	{
		ApplyMultiDamage(pevInflictor,pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity	= pEntity;
		gMultiDamage.amount		= 0;
	}

	gMultiDamage.amount += flDamage;
}

int giAmmoIndex = 0;
// Precaches the ammo and queues the ammo info for sending to clients
void AddAmmoNameToAmmoRegistry( const char *szAmmoname )
{
	// make sure it's not already in the registry
	for ( int i = 0; i < MAX_AMMO_SLOTS; i++ )
	{
		if ( !CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if ( stricmp( CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname ) == 0 )
			return; // ammo already in registry, just quite
	}


	giAmmoIndex++;
	ASSERT( giAmmoIndex < MAX_AMMO_SLOTS );
	if ( giAmmoIndex >= MAX_AMMO_SLOTS )
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
}

// Precaches the weapon and queues the weapon info for sending to clients
void UTIL_PrecacheOtherWeapon( const char *szClassname )
{
	edict_t	*pent;

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n" );
		return;
	}
	
	CBaseEntity *pEntity = CBaseEntity::Instance (VARS( pent ));

	if (pEntity)
	{
		ItemInfo II;
pEntity->Precache( );
		memset( &II, 0, sizeof II );
		if ( ((CBasePlayerItem*)pEntity)->GetItemInfo( &II ) )
		{
			CBasePlayerItem::ItemInfoArray[II.iId] = II;

			if ( II.pszAmmo1 && *II.pszAmmo1 )
			{
				AddAmmoNameToAmmoRegistry( II.pszAmmo1 );
			}

			if ( II.pszAmmo2 && *II.pszAmmo2 )
			{
				AddAmmoNameToAmmoRegistry( II.pszAmmo2 );
			}

			memset( &II, 0, sizeof II );
		}
	}

	REMOVE_ENTITY(pent);
}

// called by worldspawn
void W_Precache(void)
{
	memset( CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray) );
	memset( CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray) );
	giAmmoIndex = 0;

	ALERT( at_console, "Precaching WEAPONS...\n");

	UTIL_PrecacheOtherWeapon( "weapon_shotgun" );
	UTIL_PrecacheOtherWeapon( "weapon_shieldgun" );
	UTIL_PrecacheOtherWeapon( "weapon_autoshotgun" );
	UTIL_PrecacheOtherWeapon( "weapon_bfg" );
	UTIL_PrecacheOtherWeapon( "weapon_crowbar" );
	UTIL_PrecacheOtherWeapon( "weapon_flashbang" );
	UTIL_PrecacheOtherWeapon( "weapon_lightsaber" );
	UTIL_PrecacheOtherWeapon( "weapon_torch" );
	UTIL_PrecacheOtherWeapon( "weapon_turretkit" );
	UTIL_PrecacheOtherWeapon( "weapon_glock" );
	UTIL_PrecacheOtherWeapon( "weapon_glock_akimbo" );
	UTIL_PrecacheOtherWeapon( "weapon_mp5" );
	UTIL_PrecacheOtherWeapon( "weapon_uzi" );
	UTIL_PrecacheOtherWeapon( "weapon_uzi_akimbo" );
	UTIL_PrecacheOtherWeapon( "weapon_m16" );
	UTIL_PrecacheOtherWeapon( "weapon_u2" );
	UTIL_PrecacheOtherWeapon( "weapon_akimbogun" );
	UTIL_PrecacheOtherWeapon( "weapon_awp" );
	UTIL_PrecacheOtherWeapon( "weapon_biorifle" );
	UTIL_PrecacheOtherWeapon( "weapon_m249" );
	UTIL_PrecacheOtherWeapon( "weapon_ak74" );
	UTIL_PrecacheOtherWeapon( "weapon_minigun" );
	UTIL_PrecacheOtherWeapon( "weapon_machinegun" );
	UTIL_PrecacheOtherWeapon( "weapon_g11" );
	UTIL_PrecacheOtherWeapon( "weapon_deagle" );
	UTIL_PrecacheOtherWeapon( "weapon_usp" );
	UTIL_PrecacheOtherWeapon( "weapon_30mmsg" );
	UTIL_PrecacheOtherWeapon( "weapon_teslagun" );
	UTIL_PrecacheOtherWeapon( "weapon_pulserifle" );
	UTIL_PrecacheOtherWeapon( "weapon_froster" );
	UTIL_PrecacheOtherWeapon( "weapon_chronosceptor" );
	UTIL_PrecacheOtherWeapon( "weapon_python" );
	UTIL_PrecacheOtherWeapon( "weapon_incendiary" );
	UTIL_PrecacheOtherWeapon( "weapon_devastator" );
	UTIL_PrecacheOtherWeapon( "weapon_redeemer" );
	UTIL_PrecacheOtherWeapon( "weapon_whl" );
	UTIL_PrecacheOtherWeapon( "weapon_barett" );
	UTIL_PrecacheOtherWeapon( "weapon_smartgun" );
	UTIL_PrecacheOtherWeapon( "weapon_flamethrower" );
	UTIL_PrecacheOtherWeapon( "weapon_flakcannon" );
	UTIL_PrecacheOtherWeapon( "weapon_gauss" );
	UTIL_PrecacheOtherWeapon( "weapon_medkit" );
	UTIL_PrecacheOtherWeapon( "weapon_bandsaw" );
	UTIL_PrecacheOtherWeapon( "weapon_satellite" );
	UTIL_PrecacheOtherWeapon( "weapon_displacer" );
	UTIL_PrecacheOtherWeapon( "weapon_taucannon" );
	UTIL_PrecacheOtherWeapon( "weapon_rpg" );
	UTIL_PrecacheOtherWeapon( "weapon_crossbow" );
	UTIL_PrecacheOtherWeapon( "weapon_egon" );
	UTIL_PrecacheOtherWeapon( "weapon_gluongun" );
	UTIL_PrecacheOtherWeapon( "weapon_photongun" );
	UTIL_PrecacheOtherWeapon( "weapon_m72" );
	UTIL_PrecacheOtherWeapon( "weapon_nailgun" );
	UTIL_PrecacheOtherWeapon( "weapon_plasmarifle" );
	UTIL_PrecacheOtherWeapon( "weapon_tripmine" );
	UTIL_PrecacheOtherWeapon( "weapon_satchel" );
	UTIL_PrecacheOtherWeapon( "weapon_handgrenade");
	UTIL_PrecacheOtherWeapon( "weapon_c4");
	UTIL_PrecacheOtherWeapon( "weapon_svd" );
	UTIL_PrecacheOtherWeapon( "weapon_blaster");

	g_sModelIndexLaser = PRECACHE_MODEL((char *)g_pModelNameLaser);
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/smoke.spr");

	PRECACHE_MODEL("models/energycharge_effect.mdl");

//BOT START
	PRECACHE_SOUND("barney/teamup2.wav");
	PRECACHE_SOUND("barney/seeya.wav");
	PRECACHE_SOUND("barney/ba_raincheck.wav");
// END BOT

// Now we precache All resources, used by Half-Life (By Ghoul [BB])

	PRECACHE_MODEL("models/turret_sentry.mdl");
	PRECACHE_SOUND("turret/tu_ping.wav");
	PRECACHE_SOUND("turret/tu_materialize.wav");
	PRECACHE_SOUND("turret/tu_deploy.wav");
	PRECACHE_SOUND("turret/tu_fire.wav");
	PRECACHE_SOUND("turret/tu_fire_missile.wav");
	PRECACHE_SOUND("turret/tu_fire_ion.wav");

	PRECACHE_SOUND("game/lms_congratulation.wav");

// All models
	PRECACHE_MODEL("models/player/gordon/gordon.mdl");
	PRECACHE_MODEL("models/player/player_harmor/player_harmor.mdl");
	PRECACHE_MODEL("models/projectiles.mdl");
	PRECACHE_MODEL("models/explosions.mdl");
	PRECACHE_MODEL("models/w_items.mdl");
	PRECACHE_MODEL("models/w_shells_all.mdl");
	PRECACHE_MODEL("models/w_clips_all.mdl");
	PRECACHE_MODEL("models/w_gibs_all.mdl");

//All sprites
	PRECACHE_MODEL("sprites/rings_all.spr");

	PRECACHE_MODEL("sprites/anim_spr1.spr");
	PRECACHE_MODEL("sprites/anim_spr2.spr");
	PRECACHE_MODEL("sprites/anim_spr3.spr");
	PRECACHE_MODEL("sprites/anim_spr4.spr");
	PRECACHE_MODEL("sprites/anim_spr5.spr");
	PRECACHE_MODEL("sprites/anim_spr6.spr");
	PRECACHE_MODEL("sprites/anim_spr7.spr");
	PRECACHE_MODEL("sprites/anim_spr8.spr");
	PRECACHE_MODEL("sprites/anim_spr9.spr");
	PRECACHE_MODEL("sprites/anim_spr10.spr");
	PRECACHE_MODEL("sprites/anim_spr11.spr");
	PRECACHE_MODEL("sprites/anim_spr12.spr");
	PRECACHE_MODEL("sprites/anim_spr13.spr");

	PRECACHE_MODEL("sprites/particles_red.spr");
	PRECACHE_MODEL("sprites/particles_green.spr");
	PRECACHE_MODEL("sprites/particles_blue.spr");
	PRECACHE_MODEL("sprites/particles_violet.spr");
	PRECACHE_MODEL("sprites/particles_white.spr");
	PRECACHE_MODEL("sprites/particles_black.spr");
	PRECACHE_MODEL("sprites/particles_gibs.spr"); 
	PRECACHE_MODEL("sprites/particles_blood.spr"); 
	PRECACHE_MODEL("sprites/muzzleflash1.spr"); 
	PRECACHE_MODEL("sprites/muzzleflash2.spr"); 
	PRECACHE_MODEL("sprites/muzzleflash3.spr"); 
	PRECACHE_MODEL("sprites/plasma.spr"); 

	PRECACHE_SOUND("announce/kill_double.wav");
	PRECACHE_SOUND("announce/kill_multi.wav");
	PRECACHE_SOUND("announce/kill_monster.wav");
	PRECACHE_SOUND("announce/kill_ultra.wav");
	PRECACHE_SOUND("announce/kill_unstopable.wav");
	PRECACHE_SOUND("announce/kill_godlike.wav");

	PRECACHE_SOUND("items/money_pickup.wav");
	PRECACHE_SOUND("items/PortableHEV.wav");
	PRECACHE_SOUND("items/Cloak.wav");
	PRECACHE_SOUND("items/kevlar.wav");
	PRECACHE_SOUND("items/PortableHealthkit.wav");
	PRECACHE_SOUND("items/pt.wav");
	PRECACHE_SOUND("items/Antigrav.wav");
	PRECACHE_SOUND("items/PowerShield.wav");
	PRECACHE_SOUND("items/smallmedkit1.wav");
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
	PRECACHE_SOUND("items/suitcharge1.wav");
	PRECACHE_SOUND("items/suitchargeno1.wav");
	PRECACHE_SOUND("items/suitchargeok1.wav");
	PRECACHE_SOUND("items/flashcharger_run.wav");
	PRECACHE_SOUND("items/flashcharger_start.wav");
	PRECACHE_SOUND("items/flashcharger_no.wav");
	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");
	PRECACHE_SOUND("items/item_respawn.wav");
	PRECACHE_SOUND("items/weapon_respawn.wav");
	PRECACHE_SOUND("items/ammo_respawn.wav");
	PRECACHE_SOUND("items/gunpickup2.wav");
	PRECACHE_SOUND("items/gunpickup1.wav");
	PRECACHE_SOUND("items/flashlight1.wav");
	PRECACHE_SOUND("items/flashbattery.wav");
	PRECACHE_SOUND("items/longjump.wav");
	PRECACHE_SOUND("items/harmor.wav");
	PRECACHE_SOUND("items/weapondrop1.wav");

	PRECACHE_SOUND("weapons/grenade_hit1.wav");
	PRECACHE_SOUND("weapons/grenade_hit2.wav");
	PRECACHE_SOUND("weapons/ric1.wav");
	PRECACHE_SOUND("weapons/ric2.wav");
	PRECACHE_SOUND("weapons/ric3.wav");

	PRECACHE_SOUND("debris/concrete_impact_bullet1.wav");// hit concrete surface
	PRECACHE_SOUND("debris/concrete_impact_bullet2.wav");
	PRECACHE_SOUND("debris/concrete_impact_bullet3.wav");
	PRECACHE_SOUND("debris/concrete_impact_bullet4.wav");
	PRECACHE_SOUND("debris/metal_impact_bullet1.wav");// hit metall surface
	PRECACHE_SOUND("debris/metal_impact_bullet2.wav");
	PRECACHE_SOUND("debris/metal_impact_bullet3.wav");
	PRECACHE_SOUND("debris/metal_impact_bullet4.wav");
	PRECACHE_SOUND("debris/wood_impact_bullet1.wav");// hit wood surface
	PRECACHE_SOUND("debris/wood_impact_bullet2.wav");
	PRECACHE_SOUND("debris/wood_impact_bullet3.wav");
	PRECACHE_SOUND("debris/wood_impact_bullet4.wav");
	PRECACHE_SOUND("debris/glass_impact_bullet1.wav");// hit glass surface
	PRECACHE_SOUND("debris/glass_impact_bullet2.wav");
	PRECACHE_SOUND("debris/glass_impact_bullet3.wav");
	PRECACHE_SOUND("debris/glass_impact_bullet4.wav");
	PRECACHE_SOUND("debris/tile_impact_bullet1.wav");// hit tile surface
	PRECACHE_SOUND("debris/tile_impact_bullet2.wav");
	PRECACHE_SOUND("debris/tile_impact_bullet3.wav");
	PRECACHE_SOUND("debris/tile_impact_bullet4.wav");

	PRECACHE_SOUND("debris/bustflesh1.wav");
	PRECACHE_SOUND("debris/bustflesh2.wav");
	PRECACHE_SOUND("debris/bustmetal1.wav");
	PRECACHE_SOUND("debris/bustmetal2.wav");
	PRECACHE_SOUND("debris/bustcrate1.wav");
	PRECACHE_SOUND("debris/bustcrate2.wav");
	PRECACHE_SOUND("debris/bustglass1.wav");
	PRECACHE_SOUND("debris/bustglass2.wav");
	PRECACHE_SOUND("debris/bustconcrete1.wav");
	PRECACHE_SOUND("debris/bustconcrete2.wav");
	PRECACHE_SOUND("debris/bustceiling.wav");  

	PRECACHE_SOUND("player/nervegas1.wav");
	PRECACHE_SOUND("player/nervegas2.wav");
	PRECACHE_SOUND("player/pl_teleport1.wav");
	PRECACHE_SOUND("player/pl_teleport2.wav");
	PRECACHE_SOUND("player/pl_teleport3.wav");
	PRECACHE_SOUND("player/sprayer.wav");			// spray paint sound for PreAlpha
	PRECACHE_SOUND("player/pl_burn.wav");		// player burn sound
	PRECACHE_SOUND("player/pl_drown.wav");		// player drawn under water sound
	PRECACHE_SOUND("player/pl_drown2.wav");		
	PRECACHE_SOUND("player/pl_drown3.wav");		
	PRECACHE_SOUND("player/pl_burn_pain1.wav");	
	PRECACHE_SOUND("player/pl_burn_pain2.wav");	
	PRECACHE_SOUND("player/pl_fallpain3.wav");		
	PRECACHE_SOUND("player/pl_cloak_noise.wav");
	PRECACHE_SOUND("player/pl_cloak_deactivate.wav");
	PRECACHE_SOUND("player/pl_antigrav_fly.wav");
	PRECACHE_SOUND("player/pl_antigrav_deactivate.wav");
	PRECACHE_SOUND("player/pl_jump.wav");
	PRECACHE_SOUND("player/pl_jump_super.wav");
	PRECACHE_SOUND("player/pl_shield_impact1.wav");
	PRECACHE_SOUND("player/pl_shield_impact2.wav");
	PRECACHE_SOUND("player/pl_shield_impact3.wav");
	PRECACHE_SOUND("player/headshot1.wav");
	PRECACHE_SOUND("player/headshot2.wav");
	PRECACHE_SOUND("player/headshot3.wav");
	PRECACHE_SOUND("player/death1.wav");
	PRECACHE_SOUND("player/death2.wav");
	PRECACHE_SOUND("player/death3.wav");
	PRECACHE_SOUND("player/death4.wav");

// STEP SOUNDS
	PRECACHE_SOUND("player/pl_step_harmor1.wav");
	PRECACHE_SOUND("player/pl_step_harmor2.wav");
	PRECACHE_SOUND("player/pl_step1.wav");		// walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");
	PRECACHE_SOUND("player/pl_metal1.wav");		// walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");
	PRECACHE_SOUND("player/pl_snow1.wav");		// walk on snow
	PRECACHE_SOUND("player/pl_snow2.wav");
	PRECACHE_SOUND("player/pl_snow3.wav");
	PRECACHE_SOUND("player/pl_snow4.wav");
	PRECACHE_SOUND("player/pl_grass1.wav");		// walk on grass
	PRECACHE_SOUND("player/pl_grass2.wav");
	PRECACHE_SOUND("player/pl_grass3.wav");
	PRECACHE_SOUND("player/pl_grass4.wav");
	PRECACHE_SOUND("player/pl_sand1.wav");		// walk on sand
	PRECACHE_SOUND("player/pl_sand2.wav");
	PRECACHE_SOUND("player/pl_sand3.wav");
	PRECACHE_SOUND("player/pl_sand4.wav");
	PRECACHE_SOUND("player/pl_dirt1.wav");		// walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");
	PRECACHE_SOUND("player/pl_grate1.wav");		// walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");		
	PRECACHE_SOUND("player/pl_grate3.wav");		
	PRECACHE_SOUND("player/pl_grate4.wav");
	PRECACHE_SOUND("player/pl_slosh1.wav");		// walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");
	PRECACHE_SOUND("player/pl_swim1.wav");		// breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");
	PRECACHE_SOUND("player/pl_ladder1.wav");	// climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");
	PRECACHE_SOUND("player/pl_wade1.wav");		// wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");		
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND( "buttons/spark1.wav" );
	PRECACHE_SOUND( "buttons/spark2.wav" );
	PRECACHE_SOUND( "buttons/spark3.wav" );
	PRECACHE_SOUND( "buttons/spark4.wav" );
	PRECACHE_SOUND( "buttons/spark5.wav" );
	PRECACHE_SOUND( "buttons/spark6.wav" );
	PRECACHE_SOUND("buttons/button10.wav");
	PRECACHE_SOUND("buttons/button2.wav");

	PRECACHE_SOUND("common/null.wav");				// clears sound channels
	PRECACHE_SOUND("common/bodysplat.wav");		               
	PRECACHE_SOUND("common/bodydrop3.wav" );// dead bodies hitting the ground (animation events)
	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");
}

TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] = 
{
	DEFINE_FIELD( CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBasePlayerItem, m_pNext, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBasePlayerItem, m_iId, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CBasePlayerItem, CBaseAnimating );


TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] = 
{
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iClip, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iDefaultAmmo2, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iLastSkin, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_iLastBody, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayerWeapon, m_flTimeUpdate, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CBasePlayerWeapon, CBasePlayerItem );


void CBasePlayerItem :: SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16); 
}

//=========================================================
// Sets up movetype, size, solidtype for a new weapon. 
//=========================================================
void CBasePlayerItem :: FallInit( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	pev->sequence = 1;
	pev->scale = 1.2;

	modelindexsave=0;
	modelsave=iStringNull;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0) );

	SetTouch( DefaultTouch );
	SetThink( FallThink );

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerItem::FallThink ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if ( pev->flags & FL_ONGROUND )
	{
		pev->angles.x = 0;
		pev->angles.z = 0;
		Materialize(); 
	}
}

//=========================================================
// Materialize - make a CBasePlayerItem visible and tangible
//=========================================================
void CBasePlayerItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/weapon_respawn.wav", 1, ATTN_NORM, 0, 100 );
		pev->effects &= ~EF_NODRAW;
	}

	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin( pev, pev->origin );// link into world.
	SetTouch (DefaultTouch);
	SetThink (NULL);

}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerItem::AttemptToMaterialize( void )
{
	float time = g_pGameRules->FlWeaponTryRespawn( this );

	if ( time == 0 )
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CBasePlayerItem :: CheckRespawn ( void )
{
	switch ( g_pGameRules->WeaponShouldRespawn( this ) )
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CBasePlayerItem::Respawn( void )
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create( (char *)STRING( pev->classname ), g_pGameRules->VecWeaponRespawnSpot( this ), pev->angles, pev->owner );

	if ( pNewWeapon )
	{
		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch( NULL );// no touch
		pNewWeapon->SetThink( AttemptToMaterialize );

		DROP_TO_FLOOR ( ENT(pev) );

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime( this );
	}
	else
	{
		ALERT ( at_console, "Respawn failed to create %s!\n", STRING( pev->classname ) );
	}

	return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// can I have this?
	if ( !g_pGameRules->CanHavePlayerItem( pPlayer, this ) )
	{
		return;
	}

	if (pOther->AddPlayerItem( this ))
	{
		switch(RANDOM_LONG(0,1))
		{
		case 0:		EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup1.wav", 1, ATTN_NORM );	break;
		case 1:		EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );	break;
		}
	}

	SUB_UseTargets( pOther, USE_TOGGLE, 0 ); // UNDONE: when should this happen?
}

BOOL CanAttack( float attack_time, float curtime, BOOL isPredicted )
{
	if ( 1 )
	{
		return ( attack_time <= curtime ) ? TRUE : FALSE;
	}
	else
	{
		return ( attack_time <= 0.0 ) ? TRUE : FALSE;
	}
}

void CBasePlayerWeapon::CompleteReload( void )
{
		// complete the reload. 
		int j = min( iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);	

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

		m_pPlayer->TabulateAmmo();
		m_fInReload = FALSE;
}

void CBasePlayerWeapon::ItemPostFrame( void )
{
	RestoreBody();

	if ( ShouldWeaponIdle() )
	{
		WeaponIdle();
	}

	if ((m_fInReload) && ( m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase() ) )
	{
		CompleteReload();
	}

	if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack( m_flNextPrimaryAttack, gpGlobals->time, 0 ) && (m_pPlayer->pev->button & IN_ATTACK2) && CanAttack( m_flNextSecondaryAttack, gpGlobals->time, 0 ) )
	{
		if ( (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()]) || ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]) ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		DoubleAttack();
	}

	else if ((m_pPlayer->pev->button & IN_ATTACK2) && CanAttack( m_flNextSecondaryAttack, gpGlobals->time, 0 ) )
	{
		if ( pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack( m_flNextPrimaryAttack, gpGlobals->time, 0 ) )
	{
		if ( (m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		PrimaryAttack();
	}
	else if ( m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if ( !(m_pPlayer->pev->button & (IN_ATTACK|IN_ATTACK2) ) )
	{
		// no fire buttons down
		m_fFireOnEmpty = FALSE;

		if ( !IsUseable() && m_flNextPrimaryAttack < gpGlobals->time ) 
		{
			// weapon isn't useable, switch.
			if ( !(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon( m_pPlayer, this ) )
			{
				m_flNextPrimaryAttack = gpGlobals->time + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ( m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->time && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
			{
				Reload();
				return;
			}
		}
		ResetEmptySound(); 
		WeaponIdle( );
		return;
	}
}

void CBasePlayerItem::DestroyItem( void )
{
	if ( m_pPlayer )
	{
		// if attached to a player, remove. 
		m_pPlayer->RemovePlayerItem( this );
	}
	Kill( );
}

int CBasePlayerItem::AddToPlayer( CBasePlayer *pPlayer )
{
	m_pPlayer = pPlayer;

	return TRUE;
}

void CBasePlayerItem::Drop( void )
{
	SetTouch( NULL );
	SetThink(SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Kill( void )
{
	SetTouch( NULL );
	SetThink(SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Holster( )
{ 
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerItem::AttachToPlayer ( CBasePlayer *pPlayer )
{
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW; // ??

	if(!modelindexsave)
	{
	modelindexsave=pev->modelindex;
	modelsave=pev->model;
	}

	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + .1;
	SetTouch( NULL );
	SetThink(NULL);
}

// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
int CBasePlayerWeapon::AddDuplicate( CBasePlayerItem *pOriginal )
{
	if (m_iDefaultAmmo | m_iDefaultAmmo2)
	{
		return ExtractAmmo( (CBasePlayerWeapon *)pOriginal );
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo( (CBasePlayerWeapon *)pOriginal );
	}
}

void CBasePlayerWeapon :: BuyAmmo(int iCount, char *szName, int iCost )
{
	if (m_pPlayer->m_flMoneyAmount < iCost)
	{
		m_pPlayer->NoMoney();
		return;
	}

	int iIdAmmo = (m_pPlayer->GiveAmmo( iCount, szName, 999) != -1);
	if (iIdAmmo)
	{
		switch(RANDOM_LONG(0,1))
		{
			case 0:EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);break;
			case 1:EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip2.wav", 1, ATTN_NORM);break;
		}
		m_pPlayer->m_flMoneyAmount -= iCost;
	}
}

int CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerItem::AddToPlayer( pPlayer );

		if (m_iId < 32)
	pPlayer->pev->weapons |= (1<<m_iId);
		else
	pPlayer->m_iWeapons2 |= (1<<(m_iId - 32));

	if ( !m_iPrimaryAmmoType )
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex( pszAmmo1() );
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex( pszAmmo2() );
	}

	if (bResult)//generic message for all weapons
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		return AddWeapon( );
	}
	return FALSE;
}

int CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer )
{
	BOOL bSend = FALSE;
	int state = 0;

	if ( pPlayer->m_pActiveItem == this )
	{
		state = 1;
	}

	// Forcing send of all data!
	if ( !pPlayer->m_fWeapon )
	{
		bSend = TRUE;
	}
	
	// This is the current or last weapon, so the state will need to be updated
	if ( this == pPlayer->m_pActiveItem || this == pPlayer->m_pClientActiveItem )
	{
		if ( pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem )
		{
			bSend = TRUE;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if ( m_iClip != m_iClientClip ||  state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV )
	{
		bSend = TRUE;
	}

	if ( bSend )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev );
			WRITE_BYTE( state );
			WRITE_BYTE( m_iId );
			WRITE_BYTE( m_iClip );
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if ( m_pNext )
		m_pNext->UpdateClientData( pPlayer );

	return 1;
}

void CBasePlayerWeapon::SendWeaponAnim( int iAnim )
{
	m_pPlayer->pev->weaponanim = iAnim;

	MESSAGE_BEGIN( MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev );
		WRITE_BYTE( iAnim );						// sequence number
		WRITE_BYTE( pev->body );					// weaponmodel bodygroup.
	MESSAGE_END();
}

BOOL CBasePlayerWeapon :: AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry )
{
	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName, iMaxCarry );
	}
	else if (m_iClip == 0)
	{
		int i;
		i = min( m_iClip + iCount, iMaxClip ) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmo( iCount - i, szName, iMaxCarry );
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName, iMaxCarry );
	}
	
	// m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (m_pPlayer->HasPlayerItem( this ) )
		{

switch(RANDOM_LONG(0,1))
{
case 0:		EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/gunpickup1.wav", 1, ATTN_NORM );	break;
case 1:		EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );	break;
}
		}
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}


BOOL CBasePlayerWeapon :: AddSecondaryAmmo( int iCount, char *szName, int iMax )
{
	int iIdAmmo;

	iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName, iMax );

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip2.wav", 1, ATTN_NORM);
	}
	return iIdAmmo > 0 ? TRUE : FALSE;
}

//=========================================================
// IsUseable - this function determines whether or not a 
// weapon is useable by the player in its current state. 
// (does it have ammo loaded? do I have any ammo for the 
// weapon?, etc)
//=========================================================

// Used to fixed weapon select bug -Ms
BOOL CBasePlayerWeapon :: IsUseable( void )
{
	return CanDeploy();
}

BOOL CBasePlayerWeapon :: CanDeploy( void )
{
	return TRUE;
}

BOOL CBasePlayerWeapon :: DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, float fDrawTime )
{
	if (!CanDeploy( ))
		return FALSE;

	if (m_pPlayer->m_fHeavyArmor)
	m_pPlayer->pev->maxspeed = HARMOR_MAXSPEED;
	else
	m_pPlayer->pev->maxspeed = iMaxspeed();

	m_iLastSkin = -1;
	b_Restored = TRUE;//no need update if deploy

	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	strcpy( m_pPlayer->m_szAnimExtention, szAnimExt );
	SendWeaponAnim( iAnim );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDrawTime;//Custom time for deploy
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + fDrawTime + 1; //Make second delay beetwen draw and idle animation

	return TRUE;
}

BOOL CBasePlayerWeapon :: DefaultReload( int iClipSize, int iAnim, float fDelay, float fDrop )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	int j = min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);	

	if (j == 0)
		return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	SendWeaponAnim( iAnim );

	ItemInfo I;
	GetItemInfo ( &I );

	if ( m_iFiredAmmo >= I.iMaxClip )
	{
		SetThink ( ClipCasing );
		pev->nextthink = gpGlobals->time + fDrop;

		m_iFiredAmmo = 0;
	}

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	return TRUE;
}

/*=============================
Tsar: function, that drops clip
=============================*/
void CBasePlayerWeapon :: ClipCasing ( void )
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), pev->weapons );

	if (pev->weapons == CLIP_AKIMBOGUN_SG552) //hack for Akimbogun
	{
		SetThink ( ClipCasingAug );
		pev->nextthink = gpGlobals->time + 2.8;
	}
	else if (pev->weapons == CLIP_UZI_RIGHT) //hack for Uzi
	{
		SetThink ( ClipCasingUzi );
		pev->nextthink = gpGlobals->time + 3.5;
	}
	else if (pev->weapons == CLIP_NAILGUN)
	{
		SetThink ( ClipCasingNailgun );
		pev->nextthink = gpGlobals->time + 0.35;
	}
	else if (pev->weapons == CLIP_GLUONGUN)
	{
		SetThink ( ClipCasingGluongun );
		pev->nextthink = gpGlobals->time + 0.6;
	}
	else if (pev->weapons == CLIP_EGON)
	{
		SetThink ( ClipCasingEgon );
		pev->nextthink = gpGlobals->time + 0.8;
	}
	else if (pev->weapons == CLIP_CHRONOSCEPTOR)
	{
		SetThink ( ClipCasingChronosceptor );
		pev->nextthink = gpGlobals->time + 1.2;
	}
	if (pev->weapons == CLIP_MACHINEGUN)
	{
		SetThink ( ClipCasingMachinegun );
		pev->nextthink = gpGlobals->time + 2.8;
	}
}

void CBasePlayerWeapon :: ClipCasingAug ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_AKIMBOGUN_AUG);
}

void CBasePlayerWeapon :: ClipCasingMachinegun ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_MACHINEGUN_LEFT);
}

void CBasePlayerWeapon :: ClipCasingUzi ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_UZI_LEFT);
}

void CBasePlayerWeapon :: ClipCasingNailgun ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_NAILGUN_LEFT);
}

void CBasePlayerWeapon :: ClipCasingGluongun ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_GLUONGUN_LEFT);
}

void CBasePlayerWeapon :: ClipCasingEgon ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_EGON_MIDDLE);
	SetThink ( ClipCasingEgonLast );
	pev->nextthink = gpGlobals->time + 0.8;
}

void CBasePlayerWeapon :: ClipCasingEgonLast ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_EGON_LEFT);
}

void CBasePlayerWeapon :: ClipCasingChronosceptor ( void )
{
	FX_BrassClip( m_pPlayer->GetGunPosition(), m_pPlayer->pev->v_angle, m_pPlayer->entindex(), CLIP_CHRONOSCEPTOR_LEFT);
}

BOOL CBasePlayerWeapon :: PlayEmptySound( int iSoundType )
{
	if (m_iPlayEmptySound)
	{
		switch (iSoundType)
		{
			case 0:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ammo_out.wav", 0.8, ATTN_NORM);
			break;
			case 1:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "buttons/button10.wav", 0.8, ATTN_NORM);
			break;
			case 2:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "buttons/spark6.wav", 0.8, ATTN_NORM);
			break;
			case 3:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 0.8, ATTN_NORM);
			break;
			case 4:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/bfg_dryfire.wav", 0.8, ATTN_NORM);
			break;
			case 5:
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/psp_blast.wav", 0.3, ATTN_NORM);
			break;
		}
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CBasePlayerWeapon :: RestoreBody ( void )
{
	if(!b_Restored )
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMSGManager, NULL, m_pPlayer->pev);
		WRITE_BYTE( MSG_BODY );
		WRITE_BYTE( pev->body );
		WRITE_ELSE(1+1);
		MESSAGE_END();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		b_Restored = TRUE;
}

//update weapon skin
	if( m_iLastSkin != pev->skin)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMSGManager, NULL, m_pPlayer->pev);
		WRITE_BYTE( MSG_SKIN );
		WRITE_BYTE( pev->skin );
		WRITE_ELSE(1+1);
		MESSAGE_END();

		m_iLastSkin = pev->skin;
	}
}

void CBasePlayerWeapon :: ResetEmptySound( void )
{
	m_iPlayEmptySound = 1;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::PrimaryAmmoIndex( void )
{
	return m_iPrimaryAmmoType;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::SecondaryAmmoIndex( void )
{
	return -1;
}

void CBasePlayerWeapon::Holster( )
{ 
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;
}

//====================================================================
//			Laser pointer target
//====================================================================

LINK_ENTITY_TO_CLASS( laser_spot, CLaserSpot );
LINK_ENTITY_TO_CLASS( laser_dot, CLaserSpot );

CLaserSpot *CLaserSpot::CreateSpot( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();
	pSpot->pev->classname = MAKE_STRING("laser_dot");
	return pSpot;
}

CLaserSpot *CLaserSpot::CreateSpotNailgun( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->SpawnNailgunspot();
	pSpot->pev->classname = MAKE_STRING("laser_dot");
	return pSpot;
}

CLaserSpot *CLaserSpot::CreateSpotDGlock( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->SpawnDGlockspot();
	pSpot->pev->classname = MAKE_STRING("laser_dot");
	return pSpot;
}

CLaserSpot *CLaserSpot::CreateSpotEagle( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->SpawnEaglespot();
	pSpot->pev->classname = MAKE_STRING("laser_dot");
	return pSpot;
}

CLaserSpot *CLaserSpot::CreateSpotRpg( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();
	pSpot->pev->classname = MAKE_STRING("laser_spot");
	return pSpot;
}

void CLaserSpot::SpawnEaglespot( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;
	pev->scale = 1;
	pev->frame = 14;
	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/particles_red.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

void CLaserSpot::SpawnDGlockspot( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;
	pev->scale = 1;
	pev->frame = 15;
	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/particles_red.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

void CLaserSpot::SpawnNailgunspot( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;
	pev->scale = 0.5;
	pev->frame = 13;
	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/particles_red.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

void CLaserSpot::Spawn( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;
	pev->scale = 0.05;
	pev->frame = 0;
	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/particles_red.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

void CLaserSpot::Suspend( float flSuspendTime )
{
	pev->effects |= EF_NODRAW;
	
	// -1 means suspend indefinitely
	if (flSuspendTime == -1)
	{
		SetThink( NULL );
	}
	else
	{
		SetThink(&CLaserSpot:: Revive );
	pev->nextthink = gpGlobals->time + flSuspendTime;
	}
}

void CLaserSpot::Revive( void )
{
	pev->effects &= ~EF_NODRAW;
	SetThink( NULL );
}

//=========================================================
// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for 
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in 
// the weapon clip comes along. 
//=========================================================
int CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon )
{
	int iReturn;

	if ( pszAmmo1() != NULL )
	{
		iReturn = pWeapon->AddPrimaryAmmo( m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1() );
		m_iDefaultAmmo = 0;
	}

	if ( pszAmmo2() != NULL )
	{
		iReturn = pWeapon->AddSecondaryAmmo(m_iDefaultAmmo2, (char *)pszAmmo2(), iMaxAmmo2() );
		m_iDefaultAmmo2 = 0;
	}
	return iReturn;
}

//=========================================================
// called by the new item's class with the existing item as parameter
//=========================================================
int CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon )
{
	int			iAmmo;

	if ( m_iClip == WEAPON_NOCLIP )
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}
	
	return pWeapon->m_pPlayer->GiveAmmo( iAmmo, (char *)pszAmmo1(), iMaxAmmo1() ); // , &m_iPrimaryAmmoType
}
	
//=========================================================
// RetireWeapon - no more ammo for this gun, put it away.
//=========================================================
void CBasePlayerWeapon::RetireWeapon( void )
{
	// first, no viewmodel at all.
	m_pPlayer->pev->viewmodel = iStringNull;
	m_pPlayer->pev->weaponmodel = iStringNull;

	g_pGameRules->GetNextBestWeapon( m_pPlayer, this );
}

//*********************************************************
// weaponbox code:
//*********************************************************

LINK_ENTITY_TO_CLASS( weaponbox, CWeaponBox );

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] = 
{
	DEFINE_ARRAY( CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS ),
	DEFINE_ARRAY( CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_SLOTS ),
	DEFINE_ARRAY( CWeaponBox, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES ),
	DEFINE_FIELD( CWeaponBox, m_cAmmoTypes, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CWeaponBox, CBaseEntity );

//=========================================================
//=========================================================
void CWeaponBox :: KeyValue( KeyValueData *pkvd )
{
	if ( m_cAmmoTypes < MAX_AMMO_SLOTS )
	{
		PackAmmo( ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue) );
		m_cAmmoTypes++;// count this new ammo type.
		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT ( at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_SLOTS );
	}
}

//=========================================================
// CWeaponBox - Spawn 
//=========================================================
void CWeaponBox::Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE; 
	pev->gravity = 1; 
	pev->friction = 0.8; 
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	SET_MODEL( ENT(pev), "models/w_items.mdl");
}

//=========================================================
// CWeaponBox - Kill - the think function that removes the
// box from the world.
//=========================================================
void CWeaponBox::Kill( void )
{
	CBasePlayerItem *pWeapon;
	int i;

	// destroy the weapons
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pWeapon = m_rgpPlayerItems[ i ];

		while ( pWeapon )
		{
			pWeapon->SetThink(SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1;
			pWeapon = pWeapon->m_pNext;
		}
	}

	// remove the box
	UTIL_Remove( this );
}

//=========================================================
// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.
//=========================================================
void CWeaponBox::Touch( CBaseEntity *pOther )
{
	if (pev->flags & FL_ONGROUND)
		pev->velocity = pev->velocity * 0.1;
	else
	{
		if ( pOther->IsBSPModel() ) 
		{ 
			int pitch = 90 + RANDOM_LONG(0,30); 
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch); 
		}
	}

	pev->angles.x = pev->angles.z = 0;

	UTIL_MakeVectors ( pev->angles );

	Vector right, forward, up = Vector( 0, 0, 20 );
	TraceResult tr1, tr2;
	float angle_z;

	UTIL_TraceLine ( pev->origin + up, pev->origin - up, ignore_monsters, ENT(pev), &tr1 );
	UTIL_TraceLine ( pev->origin + gpGlobals->v_right * 5 + up, pev->origin - up + gpGlobals->v_right * 5, ignore_monsters, ENT(pev), &tr2 );

	if ( tr1.flFraction < 1.0 && tr2.flFraction < 1.0 )
	{
		up = tr1.vecPlaneNormal;
		right = ( tr2.vecEndPos - pev->origin ).Normalize ();
		forward = CrossProduct ( up, right );

		//hack: acos ranges from 0 to M_PI
		int sgn = ( gpGlobals->v_right.z > right.z ) ? 1 : -1;

		angle_z = ( acos ( DotProduct ( gpGlobals->v_right, right ) ) * 180.0f ) / M_PI;

		pev->angles = UTIL_VecToAngles ( forward );
		pev->angles.z = angle_z * sgn;
	}

	if (!(pev->flags & FL_ONGROUND ))
		return;

	if ( !pOther->IsPlayer() )
		return;

	if ( !pOther->IsAlive() )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	int i;

// go through my weapons and try to give the usable ones to the player. 
// it's important the the player be given ammo first, so the weapons code doesn't refuse 
// to deploy a better weapon that the player may pick up because he has no ammo for it.

	bool bAdded = (g_pGameRules->DeadPlayerWeapons(pPlayer) == GR_PLR_DROP_GUN_NO);//hack. if player drops only ammo, no weapon will be added to toucher anyway, and we don't need it - we need ammo

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerItem *pItem;
			CBasePlayerItem *pLink = m_rgpPlayerItems[ i ];

			// have at least one weapon in this slot
			while ( pLink )
			{
				pItem = pLink;
				pLink = pLink->m_pNext;//do not unlink!

				if ( pPlayer->AddPlayerItem( pItem ) )
				{
					bAdded = true;
				}
			}
		}
	}

	if (!bAdded) return;

// dole out ammo
	for ( i = 0 ; i < MAX_AMMO_SLOTS ; i++ )
	{
		if ( !FStringNull( m_rgiszAmmo[ i ] ) )
		{
			// there's some ammo of this type. 
			pPlayer->GiveAmmo( m_rgAmmo[ i ], (char *)STRING( m_rgiszAmmo[ i ] ), MaxAmmoCarry( m_rgiszAmmo[ i ] ) );

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[ i ] = iStringNull;
			m_rgAmmo[ i ] = 0;
		}
	}

	EMIT_SOUND( pOther->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
	SetTouch(NULL);
	UTIL_Remove(this);
}

//=========================================================
// CWeaponBox - PackWeapon: Add this weapon to the box
//=========================================================
BOOL CWeaponBox::PackWeapon( CBasePlayerItem *pWeapon )
{
	// is one of these weapons already packed in this box?
	if ( HasWeapon( pWeapon ) )
	{
		return FALSE;// box can only hold one of each weapon type
	}

	if ( pWeapon->m_pPlayer )
	{
		if ( !pWeapon->m_pPlayer->RemovePlayerItem( pWeapon ) )
		{
			// failed to unhook the weapon from the player!
			return FALSE;
		}
	}

	int iWeaponSlot = pWeapon->iItemSlot();
	
	if ( m_rgpPlayerItems[ iWeaponSlot ] )
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_pNext = m_rgpPlayerItems[ iWeaponSlot ];	
		m_rgpPlayerItems[ iWeaponSlot ] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_rgpPlayerItems[ iWeaponSlot ] = pWeapon;
		pWeapon->m_pNext = NULL;	
	}

	pWeapon->pev->spawnflags |= SF_NORESPAWN;// never respawn
	pWeapon->pev->movetype = MOVETYPE_NONE;
	pWeapon->pev->solid = SOLID_NOT;
	pWeapon->pev->effects = EF_NODRAW;

	pev->sequence = 1;
	pev->scale = 1.4;

// now we sawe model (for weapon droppings)
	if(!pWeapon->modelindexsave)
	{
	pWeapon->modelindexsave=pWeapon->pev->modelindex;
	pWeapon->modelsave=pWeapon->pev->model;
	}
	pev->modelindex=pWeapon->modelindexsave;
	pev->model=pWeapon->modelsave;
// end saves

	pWeapon->pev->owner = edict();
	pWeapon->SetThink( NULL );// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch( NULL );	

	pWeapon->m_pPlayer = NULL;

	return TRUE;
}

//=========================================================
// CWeaponBox - PackAmmo
//=========================================================
BOOL CWeaponBox::PackAmmo( int iszName, int iCount )
{
	int iMaxCarry;

	if ( FStringNull( iszName ) )
	{
		// error here
		ALERT ( at_console, "NULL String in PackAmmo!\n" );
		return FALSE;
	}
	
	iMaxCarry = MaxAmmoCarry( iszName );

	if ( iMaxCarry != -1 && iCount > 0 )
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo( iCount, (char *)STRING( iszName ), iMaxCarry );
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox - GiveAmmo
//=========================================================
int CWeaponBox::GiveAmmo( int iCount, char *szName, int iMax, int *pIndex/* = NULL*/ )
{
	int i;

	for (i = 1; i < MAX_AMMO_SLOTS && !FStringNull( m_rgiszAmmo[i] ); i++)
	{
		if (stricmp( szName, STRING( m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = min( iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_SLOTS)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING( szName );
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT( at_console, "out of named ammo slots\n");
	return i;
}

//=========================================================
// CWeaponBox::HasWeapon - is a weapon of this type already
// packed in this box?
//=========================================================
BOOL CWeaponBox::HasWeapon( CBasePlayerItem *pCheckItem )
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}
		pItem = pItem->m_pNext;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox::IsEmpty - is there anything in this box?
//=========================================================
BOOL CWeaponBox::IsEmpty( void )
{
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return FALSE;
		}
	}

	for ( i = 0 ; i < MAX_AMMO_SLOTS ; i++ )
	{
		if ( !FStringNull( m_rgiszAmmo[ i ] ) )
		{
			// still have a bit of this type of ammo
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
//=========================================================
void CWeaponBox::SetObjectCollisionBox( void )
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16); 
}