#include "InRunShopState.hpp"
#include "PlayState.hpp"
#include "core/GameApp.hpp"
#include "core/Constants.hpp"
#include "render/Renderer.hpp"
#include <algorithm>
#include <cmath>

namespace TetroShift {

InRunShopState::InRunShopState(int nextFloorNumber)
    : m_nextFloorNumber(nextFloorNumber) {}

void InRunShopState::OnEnter(GameApp& app) {
    m_animTimer = 0.0f;
    m_activeSection = 0;
    m_selectedItemIndex = 0;

    app.GetMusicManager().PlayTrack(TrackId::DraftTheme, true);
    GenerateStock(app);
}

void InRunShopState::OnExit(GameApp& /*app*/) {}

void InRunShopState::Update(GameApp& /*app*/, float dt) {
    m_animTimer += dt;
}

void InRunShopState::GenerateStock(GameApp& app) {
    // 1. Relic market: 3 cards
    m_marketCards = app.GetCardDatabase().GenerateShopChoices(3, app.GetPlayStateInventory(), m_nextFloorNumber);

    // 2. Emergency Armory Consumables
    m_armoryItems = {
        { "ITEM_SHIELD", "DEFLECTOR SHIELD CORE", "Emergency matrix shield: Prevents 1 lethal Top-Out and vaporizes top 6 rows", 40, Colors::PieceI },
        { "ITEM_BOMBS", "MATRIX BOMB INGOT", "Armory payload: Converts the next 4 spawned minos into high-explosive Bomb blocks", 20, Colors::PieceBomb },
        { "ITEM_REROLL", "QUANTUM REROLL CHARGE", "Draft modification token: Grants +1 Reroll for future card selection drafts", 15, Colors::PieceT },
        { "ITEM_CRYO", "SUB-ZERO CRYO FLASK", "Cooling module: Sinks heat to reduce fall speed by 20% on the upcoming floor sector", 25, Colors::PieceJelly }
    };
}

void InRunShopState::BuyCard(GameApp& app, size_t index) {
    if (index >= m_marketCards.size()) return;
    auto* inv = app.GetPlayStateInventoryMut();
    if (!inv) return;

    auto& card = m_marketCards[index];
    if (inv->SpendCoins(card.cost)) {
        app.GetSoundSynth().PlayLevelUp();
        app.ApplyDraftCard(card);
        m_marketCards.erase(m_marketCards.begin() + static_cast<ptrdiff_t>(index));
        if (m_selectedItemIndex >= static_cast<int>(m_marketCards.size())) {
            m_selectedItemIndex = std::max(0, static_cast<int>(m_marketCards.size()) - 1);
        }
    } else {
        app.GetSoundSynth().PlayMenuBack();
    }
}

void InRunShopState::PurgeCard(GameApp& app, size_t index) {
    auto* inv = app.GetPlayStateInventoryMut();
    if (!inv) return;

    const auto& passives = inv->GetPassives();
    const auto& actives = inv->GetActives();
    const size_t totalCards = passives.size() + actives.size();
    if (index >= totalCards) return;

    const int purgeCost = 25;
    if (inv->SpendCoins(purgeCost)) {
        std::string cardIdToRemove;
        if (index < passives.size()) {
            cardIdToRemove = passives[index].id;
        } else {
            cardIdToRemove = actives[index - passives.size()].id;
        }

        inv->RemoveCard(cardIdToRemove);
        app.GetSoundSynth().PlayLineClear(4);
    } else {
        app.GetSoundSynth().PlayMenuBack();
    }
}

void InRunShopState::UpgradeCard(GameApp& app, size_t index) {
    auto* inv = app.GetPlayStateInventoryMut();
    if (!inv) return;

    const auto& passives = inv->GetPassives();
    const auto& actives = inv->GetActives();
    const size_t totalCards = passives.size() + actives.size();
    if (index >= totalCards) return;

    const int upgradeCost = 35;
    if (inv->SpendCoins(upgradeCost)) {
        std::string cardIdToUpgrade;
        if (index < passives.size()) {
            cardIdToUpgrade = passives[index].id;
        } else {
            cardIdToUpgrade = actives[index - passives.size()].id;
        }

        inv->UpgradeCard(cardIdToUpgrade);
        app.GetSoundSynth().PlayLevelUp();
    } else {
        app.GetSoundSynth().PlayMenuBack();
    }
}

void InRunShopState::BuyConsumable(GameApp& app, size_t index) {
    if (index >= m_armoryItems.size()) return;
    auto* inv = app.GetPlayStateInventoryMut();
    if (!inv) return;

    const auto& item = m_armoryItems[index];
    if (inv->SpendCoins(item.cost)) {
        app.GetSoundSynth().PlayCardSelect();
        if (item.id == "ITEM_SHIELD") {
            inv->AddShield(1);
        } else if (item.id == "ITEM_BOMBS") {
            inv->AddBombCharges(4);
        } else if (item.id == "ITEM_REROLL") {
            inv->AddRerolls(1);
        } else if (item.id == "ITEM_CRYO") {
            inv->SetCryoBuff(true);
        }
    } else {
        app.GetSoundSynth().PlayMenuBack();
    }
}

void InRunShopState::CloseShop(GameApp& app) {
    app.GetSoundSynth().PlayCardSelect();
    app.GetStateManager().PopOverlay(app);
}

void InRunShopState::HandleInput(GameApp& app) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        // If Enter/Space on deploy or default:
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseShop(app);
            return;
        }
    }

    // Station tab navigation [TAB / 1 / 2 / 3 / 4]
    if (IsKeyPressed(KEY_TAB)) {
        m_activeSection = (m_activeSection + 1) % 4;
        m_selectedItemIndex = 0;
        app.GetSoundSynth().PlayMenuToggle();
    }
    if (IsKeyPressed(KEY_ONE)) { m_activeSection = 0; m_selectedItemIndex = 0; app.GetSoundSynth().PlayMenuToggle(); }
    if (IsKeyPressed(KEY_TWO)) { m_activeSection = 1; m_selectedItemIndex = 0; app.GetSoundSynth().PlayMenuToggle(); }
    if (IsKeyPressed(KEY_THREE)) { m_activeSection = 2; m_selectedItemIndex = 0; app.GetSoundSynth().PlayMenuToggle(); }
    if (IsKeyPressed(KEY_FOUR)) { m_activeSection = 3; m_selectedItemIndex = 0; app.GetSoundSynth().PlayMenuToggle(); }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        m_selectedItemIndex = std::max(0, m_selectedItemIndex - 1);
        app.GetSoundSynth().PlayMove();
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        m_selectedItemIndex++;
        app.GetSoundSynth().PlayMove();
    }

    // Action Key (SPACE/ENTER)
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        if (m_activeSection == 0) BuyCard(app, static_cast<size_t>(m_selectedItemIndex));
        else if (m_activeSection == 1) PurgeCard(app, static_cast<size_t>(m_selectedItemIndex));
        else if (m_activeSection == 2) BuyConsumable(app, static_cast<size_t>(m_selectedItemIndex));
        else if (m_activeSection == 3) UpgradeCard(app, static_cast<size_t>(m_selectedItemIndex));
    }
}

void InRunShopState::Render(GameApp& app) {
    // 1. Dark frosted backdrop
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(Colors::BgDark, 0.88f));

    Vector2 mousePos = GetMousePosition();

    // 2. Header banner
    std::string headerStr = "SECTOR " + std::to_string(m_nextFloorNumber) + " // INTERMEDIATE BLACK MARKET";
    DrawText(headerStr.c_str(), 60, 32, 22, Colors::TextAccent);
    DrawText("Tactical supply station - Spend run credits before entering the matrix", 60, 60, 12, Colors::TextDim);

    // Coins Pill on top right
    const auto& inv = app.GetPlayStateInventory();
    Rectangle coinPill = { WINDOW_WIDTH - 240.0f, 32.0f, 180.0f, 40.0f };
    DrawRectangleRounded(coinPill, 0.25f, 4, Fade(Colors::PieceGold, 0.15f));
    DrawRectangleLinesEx(coinPill, 1.2f, Colors::PieceGold);
    DrawText("RUN CREDITS:", static_cast<int>(coinPill.x + 14.0f), static_cast<int>(coinPill.y + 13.0f), 11, Colors::TextDim);
    std::string credStr = "$" + std::to_string(inv.GetCoins());
    DrawText(credStr.c_str(), static_cast<int>(coinPill.x + 95.0f), static_cast<int>(coinPill.y + 11.0f), 16, Colors::TextGold);

    // 3. Navigation Station Tabs
    const char* tabs[4] = { "[1] RELIC MARKET", "[2] PURGE PROTOCOL", "[3] EMERGENCY ARMORY", "[4] OVERCHARGE" };
    float tabX = 60.0f;
    float tabY = 95.0f;
    float tabW = (WINDOW_WIDTH - 120.0f) * 0.25f;

    for (int t = 0; t < 4; ++t) {
        Rectangle tRect = { tabX + static_cast<float>(t) * tabW, tabY, tabW - 8.0f, 38.0f };
        bool isHovered = CheckCollisionPointRec(mousePos, tRect);
        bool isSelected = (m_activeSection == t);

        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_activeSection = t;
            m_selectedItemIndex = 0;
            app.GetSoundSynth().PlayMenuToggle();
        }

        DrawRectangleRounded(tRect, 0.15f, 4, isSelected ? Fade(Colors::PieceI, 0.25f) : (isHovered ? Colors::BgPanelBorder : Colors::BgPanel));
        DrawRectangleLinesEx(tRect, isSelected ? 2.0f : 1.0f, isSelected ? Colors::PieceI : Colors::BgPanelBorder);
        DrawText(tabs[t], static_cast<int>(tRect.x + 16.0f), static_cast<int>(tRect.y + 11.0f), 13, isSelected ? Colors::PieceI : Colors::TextWhite);
    }

    // 4. Content Area
    float contentY = 155.0f;

    if (m_activeSection == 0) {
        // [1] Relic Market
        if (m_marketCards.empty()) {
            DrawText("ALL RELICS PURCHASED FOR THIS SECTOR", 120, static_cast<int>(contentY + 120.0f), 18, Colors::TextDim);
        } else {
            float cardW = 260.0f;
            float cardH = 340.0f;
            float spacing = 30.0f;
            float totalW = static_cast<float>(m_marketCards.size()) * cardW + static_cast<float>(m_marketCards.size() - 1) * spacing;
            float startX = (WINDOW_WIDTH - totalW) * 0.5f;

            for (size_t i = 0; i < m_marketCards.size(); ++i) {
                Rectangle cRect = { startX + static_cast<float>(i) * (cardW + spacing), contentY + 20.0f, cardW, cardH };
                bool isHovered = CheckCollisionPointRec(mousePos, cRect);
                bool isSelected = (m_selectedItemIndex == static_cast<int>(i));

                if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    BuyCard(app, i);
                    return;
                }

                Renderer r;
                r.SetFontManager(&app.GetFontManager());
                r.DrawCardUI(m_marketCards[i], cRect, isSelected, isHovered);

                // Buy button below card
                Rectangle buyBtn = { cRect.x + 10.0f, cRect.y + cRect.height - 48.0f, cRect.width - 20.0f, 36.0f };
                bool canAfford = (inv.GetCoins() >= m_marketCards[i].cost);
                DrawRectangleRounded(buyBtn, 0.2f, 4, canAfford ? Fade(Colors::PieceGold, 0.25f) : Colors::BgDark);
                DrawRectangleLinesEx(buyBtn, 1.2f, canAfford ? Colors::PieceGold : Colors::TextDim);
                std::string buyTxt = "PURCHASE ($" + std::to_string(m_marketCards[i].cost) + ")";
                DrawText(buyTxt.c_str(), static_cast<int>(buyBtn.x + 45.0f), static_cast<int>(buyBtn.y + 11.0f), 12, canAfford ? Colors::PieceGold : Colors::TextDim);
            }
        }
    } else if (m_activeSection == 1) {
        // [2] Purge Protocol (Remove curse or relic)
        const auto& passives = inv.GetPassives();
        const auto& actives = inv.GetActives();
        size_t total = passives.size() + actives.size();

        DrawText("SELECT A RELIC OR CURSE TO PERMANENTLY PURGE FROM MATRIX ($25 COINS):", 60, static_cast<int>(contentY), 13, Colors::TextDim);

        if (total == 0) {
            DrawText("NO CARDS IN INVENTORY TO PURGE", 120, static_cast<int>(contentY + 100.0f), 18, Colors::TextDim);
        } else {
            float rowY = contentY + 30.0f;
            float rowH = 48.0f;
            for (size_t i = 0; i < total && i < 8; ++i) {
                const Card& c = (i < passives.size()) ? passives[i] : actives[i - passives.size()];
                Rectangle rRect = { 60.0f, rowY + static_cast<float>(i) * (rowH + 10.0f), WINDOW_WIDTH - 120.0f, rowH };
                bool isHovered = CheckCollisionPointRec(mousePos, rRect);

                if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    PurgeCard(app, i);
                    return;
                }

                DrawRectangleRounded(rRect, 0.1f, 4, isHovered ? Fade(Colors::PieceBomb, 0.2f) : Colors::BgPanel);
                DrawRectangleLinesEx(rRect, 1.0f, isHovered ? Colors::PieceBomb : Colors::BgPanelBorder);
                DrawText(c.title.c_str(), static_cast<int>(rRect.x + 16.0f), static_cast<int>(rRect.y + 14.0f), 15, c.GetRarityColor());
                DrawText(c.description.c_str(), static_cast<int>(rRect.x + 240.0f), static_cast<int>(rRect.y + 16.0f), 11, Colors::TextDim);

                Rectangle purgeBtn = { rRect.x + rRect.width - 150.0f, rRect.y + 8.0f, 136.0f, 32.0f };
                DrawRectangleRounded(purgeBtn, 0.2f, 4, Fade(Colors::PieceBomb, 0.3f));
                DrawRectangleLinesEx(purgeBtn, 1.0f, Colors::PieceBomb);
                DrawText("PURGE ($25)", static_cast<int>(purgeBtn.x + 26.0f), static_cast<int>(purgeBtn.y + 9.0f), 12, Colors::TextWhite);
            }
        }
    } else if (m_activeSection == 2) {
        // [3] Emergency Armory Consumables
        float gridX = 60.0f;
        float gridY = contentY + 20.0f;
        float itemW = (WINDOW_WIDTH - 160.0f) * 0.5f;
        float itemH = 140.0f;

        for (size_t i = 0; i < m_armoryItems.size(); ++i) {
            int col = static_cast<int>(i % 2);
            int row = static_cast<int>(i / 2);
            Rectangle iRect = { gridX + static_cast<float>(col) * (itemW + 40.0f), gridY + static_cast<float>(row) * (itemH + 20.0f), itemW, itemH };
            bool isHovered = CheckCollisionPointRec(mousePos, iRect);

            if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                BuyConsumable(app, i);
                return;
            }

            DrawRectangleRounded(iRect, 0.08f, 6, isHovered ? Fade(m_armoryItems[i].color, 0.18f) : Colors::BgPanel);
            DrawRectangleLinesEx(iRect, 1.2f, isHovered ? m_armoryItems[i].color : Colors::BgPanelBorder);

            DrawText(m_armoryItems[i].title.c_str(), static_cast<int>(iRect.x + 18.0f), static_cast<int>(iRect.y + 16.0f), 16, m_armoryItems[i].color);
            DrawText(m_armoryItems[i].description.c_str(), static_cast<int>(iRect.x + 18.0f), static_cast<int>(iRect.y + 44.0f), 11, Colors::TextDim);

            // Purchase Pill
            Rectangle buyPill = { iRect.x + 18.0f, iRect.y + iRect.height - 42.0f, 160.0f, 30.0f };
            bool canAfford = (inv.GetCoins() >= m_armoryItems[i].cost);
            DrawRectangleRounded(buyPill, 0.2f, 4, canAfford ? Fade(Colors::PieceGold, 0.2f) : Colors::BgDark);
            DrawRectangleLinesEx(buyPill, 1.0f, canAfford ? Colors::PieceGold : Colors::TextDim);
            std::string costStr = "ACQUIRE ($" + std::to_string(m_armoryItems[i].cost) + ")";
            DrawText(costStr.c_str(), static_cast<int>(buyPill.x + 28.0f), static_cast<int>(buyPill.y + 8.0f), 12, canAfford ? Colors::PieceGold : Colors::TextDim);
        }
    } else if (m_activeSection == 3) {
        // [4] Overcharge Station (Upgrade Relic)
        const auto& passives = inv.GetPassives();
        const auto& actives = inv.GetActives();
        size_t total = passives.size() + actives.size();

        DrawText("SELECT A RELIC OR ABILITY TO OVERCHARGE (+50% EFFICACY / -1 COOLDOWN) ($35 COINS):", 60, static_cast<int>(contentY), 13, Colors::TextDim);

        if (total == 0) {
            DrawText("NO RELICS IN INVENTORY TO OVERCHARGE", 120, static_cast<int>(contentY + 100.0f), 18, Colors::TextDim);
        } else {
            float rowY = contentY + 30.0f;
            float rowH = 48.0f;
            for (size_t i = 0; i < total && i < 8; ++i) {
                const Card& c = (i < passives.size()) ? passives[i] : actives[i - passives.size()];
                Rectangle rRect = { 60.0f, rowY + static_cast<float>(i) * (rowH + 10.0f), WINDOW_WIDTH - 120.0f, rowH };
                bool isHovered = CheckCollisionPointRec(mousePos, rRect);

                if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    UpgradeCard(app, i);
                    return;
                }

                DrawRectangleRounded(rRect, 0.1f, 4, isHovered ? Fade(Colors::PieceGold, 0.2f) : Colors::BgPanel);
                DrawRectangleLinesEx(rRect, 1.0f, isHovered ? Colors::PieceGold : Colors::BgPanelBorder);
                DrawText(c.title.c_str(), static_cast<int>(rRect.x + 16.0f), static_cast<int>(rRect.y + 14.0f), 15, c.GetRarityColor());
                DrawText(c.description.c_str(), static_cast<int>(rRect.x + 240.0f), static_cast<int>(rRect.y + 16.0f), 11, Colors::TextDim);

                Rectangle upgBtn = { rRect.x + rRect.width - 170.0f, rRect.y + 8.0f, 156.0f, 32.0f };
                DrawRectangleRounded(upgBtn, 0.2f, 4, Fade(Colors::PieceGold, 0.3f));
                DrawRectangleLinesEx(upgBtn, 1.0f, Colors::PieceGold);
                DrawText("OVERCHARGE ($35)", static_cast<int>(upgBtn.x + 14.0f), static_cast<int>(upgBtn.y + 9.0f), 12, Colors::PieceGold);
            }
        }
    }

    // 5. Deploy / Continue Button
    Rectangle deployBtn = { WINDOW_WIDTH * 0.5f - 180.0f, WINDOW_HEIGHT - 65.0f, 360.0f, 44.0f };
    bool isDeployHovered = CheckCollisionPointRec(mousePos, deployBtn);
    if (isDeployHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        CloseShop(app);
        return;
    }

    DrawRectangleRounded(deployBtn, 0.25f, 4, isDeployHovered ? Fade(Colors::PieceI, 0.35f) : Fade(Colors::PieceI, 0.20f));
    DrawRectangleLinesEx(deployBtn, 1.5f, isDeployHovered ? WHITE : Colors::PieceI);
    std::string deployTxt = "DEPLOY TO SECTOR " + std::to_string(m_nextFloorNumber) + " [ENTER / ESC]";
    int dtw = MeasureText(deployTxt.c_str(), 14);
    DrawText(deployTxt.c_str(), static_cast<int>(deployBtn.x + (deployBtn.width - dtw) * 0.5f), static_cast<int>(deployBtn.y + 14.0f), 14, Colors::TextWhite);
}

} // namespace TetroShift
