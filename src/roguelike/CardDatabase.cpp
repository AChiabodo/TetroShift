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

    // 1. Jelly Body (Common)
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

    // 2. Titanic Mass (Rare)
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

    // 3. Midas Touch (Legendary)
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

    // 4. Explosive Grid (Rare)
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

    // 5. Laser Focus (Common)
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

    // 6. Quantum Phase (Epic)
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

    // 7. Magnetic Pull (Active Ability)
    {
        Card c;
        c.id = "CARD_MAGNET_PULL";
        c.title = "Attrazione Magnetica";
        c.description = "[Abilita Attiva - Tasto 1] (Ricarica: 4 linee): Attira tutti i blocchi orfani verso la parete destra.";
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

    // 8. Copper Wire (Common)
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

    // 9. Combo Surge (Epic)
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

    // 10. Chain Fission (Epic)
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

    // 11. Curse of Chaos (Cursed)
    {
        Card c;
        c.id = "CARD_CURSE_CHAOS";
        c.title = "Maledizione: Caos Gravitazionale";
        c.description = "[MALEDIZIONE] Moltiplicatore Punteggio +120% (x2.2), ma la velocita di caduta aumenta del 40%!";
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

    // 12. Neon Overdrive (Legendary)
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
