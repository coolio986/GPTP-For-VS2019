#pragma once

//V241 for VS2008

namespace WeaponId {
enum Enum {
  GaussRifle                                = 0x00,
  GaussRifle_JimRaynor_Marine               = 0x01,
  C10_CanisterRifle                         = 0x02,
  C10_CanisterRifle_SarahKerrigan           = 0x03,
  FragmentationGrenade                      = 0x04,
  FragmentationGrenade_JimRaynor_Vulture    = 0x05,
  SpiderMines                               = 0x06,
  TwinAutocannons                           = 0x07,
  HellfireMissilePack                       = 0x08,
  TwinAutocannons_AlanSchezar               = 0x09,
  HellfireMissilePack_AlanSchezar           = 0x0A,
  ArcliteCannon                             = 0x0B,
  ArcliteCannon_EdmundDuke                  = 0x0C,
  FusionCutter                              = 0x0D,
  FusionCutter_Harvest                      = 0x0E,
  GeminiMissiles                            = 0x0F,
  BurstLasers                               = 0x10,
  GeminiMissiles_TomKazansky                = 0x11,
  BurstLasers_TomKazansky                   = 0x12,
  ATS_LaserBattery                          = 0x13,
  ATA_LaserBattery                          = 0x14,
  ATS_LaserBattery_Norad_II_Mengsk_DuGalle  = 0x15,
  ATA_LaserBattery_Norad_II_Mengsk_DuGalle  = 0x16,
  ATS_LaserBattery_Hyperion                 = 0x17,
  ATA_LaserBattery_Hyperion                 = 0x18,
  FlameThrower                              = 0x19,
  FlameThrower_GuiMontag                    = 0x1A,
  ArcliteShockCannon                        = 0x1B,
  ArcliteShockCannon_EdmundDuke             = 0x1C,
  LongboltMissile                           = 0x1D,
  YamatoGun                                 = 0x1E,
  NuclearMissile                            = 0x1F,
  Lockdown                                  = 0x20,
  EMP_Shockwave                             = 0x21,
  Irradiate                                 = 0x22,
  Claws                                     = 0x23,
  Claws_DevouringOne                        = 0x24,
  Claws_InfestedKerrigan                    = 0x25,
  NeedleSpines                              = 0x26,
  NeedleSpines_HunterKiller                 = 0x27,
  KaiserBlades                              = 0x28,
  KaiserBlades_Torrasque                    = 0x29,
  ToxicSpores                               = 0x2A,
  Spines                                    = 0x2B,
  Spines_Harvest                            = 0x2C,
  AcidSpray_Unused                          = 0x2D,
  AcidSpore                                 = 0x2E,
  AcidSpore_Kukulza_Guardian                = 0x2F,
  GlaveWurm                                 = 0x30,
  GlaveWurm_Kukulza_Mutalisk                = 0x31,
  Venom_Unused_Defiler                      = 0x32,
  Venom_Unused_Defiler_Hero                 = 0x33,
  SeekerSpores                              = 0x34,
  SubterraneanTentacle                      = 0x35,
  Suicide_InfestedTerran                    = 0x36,
  Suicide_Scourge                           = 0x37,
  Parasite                                  = 0x38,
  SpawnBroodlings                           = 0x39,
  Ensnare                                   = 0x3A,
  DarkSwarm                                 = 0x3B,
  Plague                                    = 0x3C,
  Consume                                   = 0x3D,
  ParticleBeam                              = 0x3E,
  ParticleBeam_Harvest                      = 0x3F,
  PsiBlades                                 = 0x40,
  PsiBlades_Fenix_Zealot                    = 0x41,
  PhaseDisruptor                            = 0x42,
  PhaseDisruptor_Fenix_Dragoon              = 0x43,
  PsiAssault_Normal_Unused                  = 0x44,
  PsiAssault_Tassadar_Aldaris               = 0x45,
  PsionicShockwave                          = 0x46,
  PsionicShockwave_Tassadar_Zeratul_Archon  = 0x47,
  Unknown72                                 = 0x48,
  DualPhotonBlasters                        = 0x49,
  AntiMatterMissiles                        = 0x4A,
  DualPhotonBlasters_Mojo                   = 0x4B,
  AntiMatterMissiles_Mojo                   = 0x4C,
  PhaseDisruptorCannon                      = 0x4D,
  PhaseDisruptorCannon_Danimoth             = 0x4E,
  PulseCannon                               = 0x4F,
  STS_PhotonCannon                          = 0x50,
  STA_PhotonCannon                          = 0x51,
  Scarab                                    = 0x52,
  StasisField                               = 0x53,
  PsiStorm                                  = 0x54,
  WarpBlades_Zeratul                        = 0x55,
  WarpBlades_Dark_Templar_Hero              = 0x56,
  Missiles_Unused                           = 0x57,
  LaserBattery1_Unused                      = 0x58,
  TormentorMissiles_Unused                  = 0x59,
  Bombs_Unused                              = 0x5A,
  RaiderGun_Unused                          = 0x5B,
  LaserBattery2_Unused                      = 0x5C,
  LaserBattery3_Unused                      = 0x5D,
  DualPhotonBlasters_Unused                 = 0x5E,
  FlechetteGrenade_Unused                   = 0x5F,
  TwinAutocannons_FloorTrap                 = 0x60,
  HellfireMissilePack_WallTrap              = 0x61,
  FlameThrower_WallTrap                     = 0x62,
  HellfireMissilePack_FloorTrap             = 0x63,
  NeutronFlare                              = 0x64,
  DisruptionWeb                             = 0x65,
  Restoration                               = 0x66,
  HaloRockets                               = 0x67,
  CorrosiveAcid                             = 0x68,
  MindControl                               = 0x69,
  Feedback                                  = 0x6A,
  OpticalFlare                              = 0x6B,
  Maelstrom                                 = 0x6C,
  SubterraneanSpines                        = 0x6D,
  GaussRifle0_Unused                        = 0x6E,
  WarpBlades                                = 0x6F,
  C10_ConcussionRifle_SamirDuran            = 0x70,
  C10_ConcussionRifle_InfestedDuran         = 0x71,
  DualPhotonBlasters_Artanis                = 0x72,
  AntiMatterMissiles_Artanis                = 0x73,
  C10_ConcussionRifle_AlexeiStukov          = 0x74,
  GaussRifle1_Unused                        = 0x75,
  GaussRifle2_Unused                        = 0x76,
  GaussRifle3_Unused                        = 0x77,
  GaussRifle4_Unused                        = 0x78,
  GaussRifle5_Unused                        = 0x79,
  GaussRifle6_Unused                        = 0x7A,
  GaussRifle7_Unused                        = 0x7B,
  GaussRifle8_Unused                        = 0x7C,
  GaussRifle9_Unused                        = 0x7D,
  GaussRifle10_Unused                       = 0x7E,
  GaussRifle11_Unused                       = 0x7F,
  GaussRifle12_Unused                       = 0x80,
  GaussRifle13_Unused                       = 0x81,
  None                                      = 0x82,
};
}