#include "CardDatabase.hpp"
#include "RunManager.hpp"
#include "piece/ActivePiece.hpp"
#include "piece/PieceSpawner.hpp"
#include "grid/OrthogonalGrid.hpp"
#include <algorithm>
#include <chrono>

namespace TetroShift {

CardDatabase::CardDatabase() {
    uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    m_rng.seed(seed);
    RegisterAllCards();
}

void CardDatabase::RegisterAllCards() {
    m_cards.clear();

    // 1. Jelly Body (Common Relic)
    {
        Card c;
        c.id = "CARD_JELLY";
        c.title = "Corpo Gelatinoso";
        c.description = "Aumenta l'elasticita dei pezzi (+80%). I pezzi rimbalzano e oscillano dinamicamente all'impatto.";
        c.rarity = CardRarity::Common;
        c.category = CardCategory::PassiveRelic;
        c.cost = 15;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) ctx.runManager->SetGlobalElasticity(1.8f);
        };
        m_cards.push_back(c);
    }

    // 2. Titanic Mass (Rare Relic)
    {
        Card c;
        c.id = "CARD_TITANIC";
        c.title = "Massa Titanica";
        c.description = "I pezzi pesano il doppio (+20% velocita). Al blocco generano un potente impatto sismico.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::PassiveRelic;
        c.cost = 30;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->SetBaseFallSpeedMultiplier(1.2f);
                ctx.runManager->MultiplyScoreModifier(1.25f);
            }
        };
        m_cards.push_back(c);
    }

    // 3. Midas Touch (Legendary Relic)
    {
        Card c;
        c.id = "CARD_MIDAS";
        c.title = "Tocco di Mida";
        c.description = "Ogni 8 pezzi spawna un pezzo interamente Dorato! Pulire mino d'oro assegna +5 Monete extra ciascuno.";
        c.rarity = CardRarity::Legendary;
        c.category = CardCategory::PassiveRelic;
        c.cost = 60;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.spawner) ctx.spawner->SetMidasFrequency(8);
        };
        m_cards.push_back(c);
    }

    // 4. Explosive Grid (Rare Relic)
    {
        Card c;
        c.id = "CARD_BOMB_RAIN";
        c.title = "Reticolo Esplosivo";
        c.description = "25% di probabilita di spawnare Blocchi Bomba: pulirli innesca un'esplosione 3x3!";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::PassiveRelic;
        c.cost = 35;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.spawner) ctx.spawner->SetBombChance(0.25f);
        };
        m_cards.push_back(c);
    }

    // 5. Laser Focus (Common Relic)
    {
        Card c;
        c.id = "CARD_LASER_FOCUS";
        c.title = "Mirino Laser";
        c.description = "Rallenta la caduta base del 15% e aumenta i punti linea del +20%.";
        c.rarity = CardRarity::Common;
        c.category = CardCategory::PassiveRelic;
        c.cost = 15;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->MultiplyScoreModifier(1.20f);
                ctx.runManager->SetBaseFallSpeedMultiplier(0.85f);
            }
        };
        m_cards.push_back(c);
    }

    // 6. Quantum Phase (Epic Relic)
    {
        Card c;
        c.id = "CARD_QUANTUM_PHASE";
        c.title = "Fase Quantistica";
        c.description = "I nuovi pezzi sono intangibili per i primi 0.7 secondi, permettendo di passare oltre gli ostacoli superiori.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 45;
        c.onPieceSpawn = [](const CardContext& ctx) {
            if (ctx.activePiece) ctx.activePiece->SetGhostPhase(0.7f);
        };
        m_cards.push_back(c);
    }

    // 7. Copper Wire (Common Relic)
    {
        Card c;
        c.id = "CARD_COPPER_WIRE";
        c.title = "Filo di Rame";
        c.description = "Guadagni +2 Monete aggiuntive per ogni linea completata.";
        c.rarity = CardRarity::Common;
        c.category = CardCategory::PassiveRelic;
        c.cost = 12;
        c.onLineClear = [](const CardContext& ctx, int linesCount, bool) {
            if (ctx.runManager) {
                ctx.runManager->GetInventory().AddCoins(linesCount * 2);
            }
        };
        m_cards.push_back(c);
    }

    // 8. Combo Surge (Epic Relic)
    {
        Card c;
        c.id = "CARD_COMBO_SURGE";
        c.title = "Impeto di Combo";
        c.description = "Ogni punto combo assegna il doppio dei punti base e +1 moneta bonus.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 45;
        c.onLineClear = [](const CardContext& ctx, int, bool) {
            if (ctx.runManager && ctx.runManager->GetCombo() > 1) {
                ctx.runManager->GetInventory().AddCoins(ctx.runManager->GetCombo());
            }
        };
        m_cards.push_back(c);
    }

    // 9. Chain Fission (Epic Relic)
    {
        Card c;
        c.id = "CARD_CHAIN_FISSION";
        c.title = "Reazione a Catena";
        c.description = "Fare un TETRIS (4 linee) assegna +2000 punti immediati e 15 monete bonus.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 50;
        c.onLineClear = [](const CardContext& ctx, int, bool isTetris) {
            if (isTetris && ctx.runManager) {
                ctx.runManager->AddScore(2000, "CHAIN FISSION!");
                ctx.runManager->GetInventory().AddCoins(15);
            }
        };
        m_cards.push_back(c);
    }

    // 10. Neon Overdrive (Legendary Relic)
    {
        Card c;
        c.id = "CARD_NEON_OVERDRIVE";
        c.title = "Overdrive Neon";
        c.description = "Punteggio triplicato (x3.0) per l'intera durata della run e +30 Monete istantanee.";
        c.rarity = CardRarity::Legendary;
        c.category = CardCategory::PassiveRelic;
        c.cost = 70;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->MultiplyScoreModifier(3.0f);
                ctx.runManager->GetInventory().AddCoins(30);
            }
        };
        m_cards.push_back(c);
    }

    // 11. Chrono Catalyst (Rare Relic)
    {
        Card c;
        c.id = "CARD_CHRONO_CATALYST";
        c.title = "Cronocatalizzatore";
        c.description = "Ogni combo ≥ 2 rallenta la velocita di caduta del 40% per stabilizzare la matrice.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::PassiveRelic;
        c.cost = 32;
        c.onLineClear = [](const CardContext& ctx, int, bool) {
            if (ctx.runManager && ctx.runManager->GetCombo() >= 2) {
                ctx.runManager->SetBaseFallSpeedMultiplier(0.6f);
            }
        };
        m_cards.push_back(c);
    }

    // 12. Sub-Atomic Sand Shatter (Epic Relic)
    {
        Card c;
        c.id = "CARD_MINO_SHATTER";
        c.title = "Disgregazione Silicea";
        c.description = "Il 20% dei pezzi generati e composto da Sabbia Fluida che si disintegra e cola nei buchi della matrice.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 40;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.spawner) ctx.spawner->SetSandChance(0.20f);
        };
        m_cards.push_back(c);
    }

    // 13. Garbage Recycler (Rare Relic)
    {
        Card c;
        c.id = "CARD_GARBAGE_RECYCLER";
        c.title = "Condensatore di Spazzatura";
        c.description = "Pulire 3 o piu righe contemporaneamente converte 2 blocchi sul fondo in Oro Puro (+$10 monete).";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::PassiveRelic;
        c.cost = 28;
        c.onLineClear = [](const CardContext& ctx, int linesCount, bool) {
            if (linesCount >= 3 && ctx.grid && ctx.runManager) {
                ctx.runManager->GetInventory().AddCoins(10);
            }
        };
        m_cards.push_back(c);
    }

    // 14. Antimatter Laser Core (Legendary Relic)
    {
        Card c;
        c.id = "CARD_ANTIMATTER_CORE";
        c.title = "Reattore Antimateria";
        c.description = "Ogni Hard Drop rilascia una scarica d'antimateria che polverizza la riga piu in basso.";
        c.rarity = CardRarity::Legendary;
        c.category = CardCategory::PassiveRelic;
        c.cost = 65;
        c.onPieceLock = [](const CardContext& ctx) {
            if (ctx.grid) ctx.grid->VaporizeBottomRow();
        };
        m_cards.push_back(c);
    }

    // 15. Tachyon Mirror (Epic Relic)
    {
        Card c;
        c.id = "CARD_TACHYON_MIRROR";
        c.title = "Specchio Tachionico";
        c.description = "Sblocca il 2° slot simultaneo di Hold per conservare 2 tetramini contemporaneamente.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 45;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.spawner) ctx.spawner->SetDoubleHold(true);
        };
        m_cards.push_back(c);
    }

    // 16. Resonance Cascade (Rare Relic)
    {
        Card c;
        c.id = "CARD_RESONANCE_CASCADE";
        c.title = "Cascata di Risonanza";
        c.description = "Righe doppie e triple assegnano un bonus moltiplicatore del +150% al punteggio.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::PassiveRelic;
        c.cost = 30;
        c.onLineClear = [](const CardContext& ctx, int linesCount, bool) {
            if ((linesCount == 2 || linesCount == 3) && ctx.runManager) {
                ctx.runManager->AddScore(linesCount * 300, "RESONANCE CASCADE!");
            }
        };
        m_cards.push_back(c);
    }

    // 17. Deflector Shield Unit (Epic Relic)
    {
        Card c;
        c.id = "CARD_DEFLECTOR_SHIELD";
        c.title = "Scudo di Deflessione";
        c.description = "Fornisce +1 Scudo d'emergenza che previene la sconfitta da Top-Out polverizzando 6 righe.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::PassiveRelic;
        c.cost = 55;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) ctx.runManager->GetInventory().AddShield(1);
        };
        m_cards.push_back(c);
    }

    // 18. Kinetic Overload (Common Relic)
    {
        Card c;
        c.id = "CARD_KINETIC_OVERLOAD";
        c.title = "Sovraccarico Cinetico";
        c.description = "La velocita di caduta aumenta del +15%, ma ogni riga pulita assegna +35% punteggio extra.";
        c.rarity = CardRarity::Common;
        c.category = CardCategory::PassiveRelic;
        c.cost = 18;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->SetBaseFallSpeedMultiplier(1.15f);
                ctx.runManager->MultiplyScoreModifier(1.35f);
            }
        };
        m_cards.push_back(c);
    }

    // 19. Magnetic Pull (Active Ability, CD: 4)
    {
        Card c;
        c.id = "CARD_MAGNET_PULL";
        c.title = "Attrazione Magnetica";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 4 linee): Attira tutti i blocchi orfani verso la parete destra.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::ActiveAbility;
        c.cost = 35;
        c.maxCooldown = 4;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            auto* ortho = dynamic_cast<OrthogonalGrid*>(ctx.grid);
            if (ortho) {
                ortho->ApplyHorizontalMagneticPull(true);
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 20. Singularity Vortex (Active Ability, CD: 5)
    {
        Card c;
        c.id = "CARD_SINGULARITY_VORTEX";
        c.title = "Singolarita Istantanea";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 5 linee): Comprime tutti i blocchi verso il basso eliminando i vuoti.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::ActiveAbility;
        c.cost = 45;
        c.maxCooldown = 5;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            if (ctx.grid) {
                ctx.grid->CollapseFloatingCells();
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 21. EMP Blast (Active Ability, CD: 6)
    {
        Card c;
        c.id = "CARD_EMP_BLAST";
        c.title = "Impulso PEM";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 6 linee): Polverizza istantaneamente la riga piu in basso della matrice.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::ActiveAbility;
        c.cost = 40;
        c.maxCooldown = 6;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            if (ctx.grid) {
                ctx.grid->VaporizeBottomRow();
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 22. Alchemical Shift (Active Ability, CD: 4)
    {
        Card c;
        c.id = "CARD_ALCHEMICAL_SHIFT";
        c.title = "Trasmutazione Oro";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 4 linee): Muta il tetramino attivo in un blocco d'Oro Puro (+$20 monete al lock).";
        c.rarity = CardRarity::Legendary;
        c.category = CardCategory::ActiveAbility;
        c.cost = 55;
        c.maxCooldown = 4;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            if (ctx.runManager) {
                ctx.runManager->GetInventory().AddCoins(20);
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 23. Time Freeze (Active Ability, CD: 5)
    {
        Card c;
        c.id = "CARD_TIME_FREEZE";
        c.title = "Frattura Temporale";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 5 linee): Congela la velocita di caduta al minimo per pianificare le mosse.";
        c.rarity = CardRarity::Legendary;
        c.category = CardCategory::ActiveAbility;
        c.cost = 60;
        c.maxCooldown = 5;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            if (ctx.runManager) {
                ctx.runManager->SetBaseFallSpeedMultiplier(0.2f);
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 24. Antimatter Bomb (Active Ability, CD: 4)
    {
        Card c;
        c.id = "CARD_ANTIMATTER_BOMB";
        c.title = "Bomba Antimateria";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 4 linee): Detona un'esplosione 5x5 che vaporizza blocchi ostacoli al centro.";
        c.rarity = CardRarity::Epic;
        c.category = CardCategory::ActiveAbility;
        c.cost = 45;
        c.maxCooldown = 4;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            auto* ortho = dynamic_cast<OrthogonalGrid*>(ctx.grid);
            if (ortho && ctx.activePiece) {
                std::vector<GridCoord> destroyed;
                ortho->ExplodeArea(ctx.activePiece->GetPosition(), 2, destroyed);
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 25. Shape Replicator (Active Ability, CD: 3)
    {
        Card c;
        c.id = "CARD_CLONE_MATRIX";
        c.title = "Replicatore di Forma";
        c.description = "[Abilita Attiva - Tasto 1/2] (CD: 3 linee): Muta istantaneamente il tetramino corrente in una I-Beam perfetta.";
        c.rarity = CardRarity::Rare;
        c.category = CardCategory::ActiveAbility;
        c.cost = 35;
        c.maxCooldown = 3;
        c.currentCooldown = 0;
        c.onActiveUse = [](const CardContext& ctx) -> bool {
            if (ctx.activePiece) {
                ctx.activePiece->Spawn(TetrominoType::I, ctx.activePiece->GetPosition());
                return true;
            }
            return false;
        };
        m_cards.push_back(c);
    }

    // 26. Curse of Chaos (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_CHAOS";
        c.title = "Maledizione: Caos Gravitazionale";
        c.description = "[MALEDIZIONE] Moltiplicatore Punteggio x2.2 (+120%), ma la velocita di caduta aumenta del 40%!";
        c.rarity = CardRarity::Cursed;
        c.category = CardCategory::Curse;
        c.cost = 0;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->MultiplyScoreModifier(2.2f);
                ctx.runManager->SetBaseFallSpeedMultiplier(1.4f);
            }
        };
        m_cards.push_back(c);
    }

    // 27. Curse of Neon Blood (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_NEON_BLOOD";
        c.title = "Maledizione: Sangue di Neon";
        c.description = "[MALEDIZIONE] Punteggio moltiplicato x3.5, ma l'obiettivo di linee per piano e raddoppiato!";
        c.rarity = CardRarity::Cursed;
        c.category = CardCategory::Curse;
        c.cost = 0;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->MultiplyScoreModifier(3.5f);
                ctx.runManager->SetFloorLineTarget(ctx.runManager->GetFloorLineTarget() * 2);
            }
        };
        m_cards.push_back(c);
    }

    // 28. Curse of Quantum Flux (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_QUANTUM_FLUX";
        c.title = "Maledizione: Instabilita";
        c.description = "[MALEDIZIONE] +50 Monete immediate e +100% XP, ma i pezzi cadono con accelerazione instabile.";
        c.rarity = CardRarity::Cursed;
        c.category = CardCategory::Curse;
        c.cost = 0;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->GetInventory().AddCoins(50);
                ctx.runManager->SetBaseFallSpeedMultiplier(1.25f);
            }
        };
        m_cards.push_back(c);
    }

    // 29. Curse of Titan Weight (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_TITAN_WEIGHT";
        c.title = "Maledizione: Peso Titanico";
        c.description = "[MALEDIZIONE] Monete guadagnate triplicate (x3.0), ma i pezzi cadono a velocita estrema (+50%).";
        c.rarity = CardRarity::Cursed;
        c.category = CardCategory::Curse;
        c.cost = 0;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) {
                ctx.runManager->SetBaseFallSpeedMultiplier(1.5f);
                ctx.runManager->MultiplyScoreModifier(2.0f);
            }
        };
        m_cards.push_back(c);
    }

    // 30. Void Pact (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_VOID_PACT";
        c.title = "Maledizione: Patto con l'Abisso";
        c.description = "[MALEDIZIONE] Guadagni +100 Monete immediate, ma ogni piano inizia con 2 Garbage Rows gia presenti!";
        c.rarity = CardRarity::Cursed;
        c.category = CardCategory::Curse;
        c.cost = 0;
        c.onAcquire = [](const CardContext& ctx) {
            if (ctx.runManager) ctx.runManager->GetInventory().AddCoins(100);
            if (ctx.grid) {
                ctx.grid->PushGarbageRow(4, CellType::HeavyIron, Colors::PieceIron);
                ctx.grid->PushGarbageRow(6, CellType::HeavyIron, Colors::PieceIron);
            }
        };
        m_cards.push_back(c);
    }
}

const Card* CardDatabase::FindCardById(const std::string& id) const {
    for (const auto& card : m_cards) {
        if (card.id == id) return &card;
    }
    return nullptr;
}

std::vector<Card> CardDatabase::GenerateDraftChoices(size_t count, const Inventory& currentInventory, int floorNumber) {
    std::vector<Card> available;
    for (const auto& card : m_cards) {
        // Exclude active abilities if already owned
        if (!currentInventory.HasCard(card.id)) {
            available.push_back(card);
        }
    }

    if (available.empty()) {
        return {};
    }

    // Weighted random selection based on rarity and floor
    std::vector<Card> selected;
    std::shuffle(available.begin(), available.end(), m_rng);

    for (const auto& card : available) {
        if (selected.size() >= count) break;

        // Give slight bonus to higher rarities on higher floors
        std::uniform_int_distribution<int> rollDist(1, 100);
        int roll = rollDist(m_rng);

        bool accept = true;
        if (card.rarity == CardRarity::Legendary && roll > (15 + floorNumber * 3)) {
            accept = false;
        } else if (card.rarity == CardRarity::Epic && roll > (40 + floorNumber * 4)) {
            accept = false;
        }

        if (accept || available.size() <= count) {
            selected.push_back(card);
        }
    }

    // Fallback if not enough were picked
    for (const auto& card : available) {
        if (selected.size() >= count) break;
        if (std::find_if(selected.begin(), selected.end(), [&](const Card& c) { return c.id == card.id; }) == selected.end()) {
            selected.push_back(card);
        }
    }

    return selected;
}

std::vector<Card> CardDatabase::GenerateShopChoices(size_t count, const Inventory& currentInventory, int floorNumber) {
    return GenerateDraftChoices(count, currentInventory, floorNumber);
}

} // namespace TetroShift
