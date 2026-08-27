#pragma once
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include "grid/GridCoord.hpp"

namespace TetroShift {

// Forward declarations
enum class TetrominoType : uint8_t;

// Event Definitions
struct EventPieceSpawn {
    TetrominoType type;
    int x;
    int y;
};

struct EventPieceMove {
    int dx;
    int dy;
    bool success;
};

struct EventPieceRotate {
    int newRotation;
    bool wallKicked;
};

struct EventPieceHardDrop {
    int linesDropped;
    int landingY;
};

struct EventPieceLock {
    TetrominoType type;
    std::vector<GridCoord> lockedCells;
};

struct EventLineClear {
    int linesCount;
    std::vector<int> rows;
    int combo;
    bool isTetris;
    int scoreAdded;
};

struct EventScoreGained {
    int points;
    std::string reason;
};

struct EventCoinsGained {
    int amount;
    std::string source;
};

struct EventCardDraftTriggered {
    int floorNumber;
    int choicesCount;
};

struct EventCardAcquired {
    std::string cardId;
};

struct EventLevelUp {
    int oldLevel;
    int newLevel;
};

struct EventGameOver {
    int finalScore;
    int finalFloor;
    int totalLines;
};

// Generic type-safe EventBus
class EventBus {
public:
    using HandlerId = size_t;

    template <typename TEvent>
    using EventHandler = std::function<void(const TEvent&)>;

    EventBus() = default;
    ~EventBus() = default;

    // Non-copyable, movable
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) noexcept = default;
    EventBus& operator=(EventBus&&) noexcept = default;

    template <typename TEvent>
    HandlerId Subscribe(EventHandler<TEvent> handler) {
        const std::type_index typeIndex = std::type_index(typeid(TEvent));
        auto wrapper = std::make_unique<HandlerWrapper<TEvent>>(std::move(handler));
        HandlerId id = m_nextHandlerId++;
        m_handlers[typeIndex].emplace_back(id, std::move(wrapper));
        return id;
    }

    template <typename TEvent>
    void Publish(const TEvent& event) {
        const std::type_index typeIndex = std::type_index(typeid(TEvent));
        auto it = m_handlers.find(typeIndex);
        if (it != m_handlers.end()) {
            for (auto& [id, wrapper] : it->second) {
                if (wrapper) {
                    static_cast<HandlerWrapper<TEvent>*>(wrapper.get())->Invoke(event);
                }
            }
        }
    }

    void Clear() {
        m_handlers.clear();
    }

private:
    struct IHandlerWrapper {
        virtual ~IHandlerWrapper() = default;
    };

    template <typename TEvent>
    struct HandlerWrapper : public IHandlerWrapper {
        EventHandler<TEvent> handler;
        explicit HandlerWrapper(EventHandler<TEvent> h) : handler(std::move(h)) {}
        void Invoke(const TEvent& e) const {
            if (handler) handler(e);
        }
    };

    HandlerId m_nextHandlerId = 1;
    std::unordered_map<std::type_index, std::vector<std::pair<HandlerId, std::unique_ptr<IHandlerWrapper>>>> m_handlers;
};

} // namespace TetroShift
