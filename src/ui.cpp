#include "ui.hpp"

UI::UI(sf::RenderWindow& window, const sf::Vector2f& windowSize) :
    window(window),
    windowSize(windowSize)
{}

void UI::updateUI() {
    resizeResignationButton();

    resizeResignationConfirmationLayout();

    resizeLayoutButtons();

    resizePromoLayout();
    recalculateMoveHistorySize();

    resizeMainMenuLayout();
    
    resizePrePlayLayout();
    resizeSettingsLayout();

    resizeGameOverLayout();
}

void UI::handleResize(const unsigned int windowWidth, const unsigned int windowHeight) {
    renderer.setInfo(windowWidth, windowHeight);
    recalibrateViews(windowWidth, windowHeight);
}

void UI::recalibrateViews(const unsigned int windowWidth, const unsigned int windowHeight) {
    const sf::Vector2f centerOfWindow = { static_cast<float>(windowWidth) / 2.f, static_cast<float>(windowHeight) / 2.f };
    const sf::Vector2f sizeOfWindow = static_cast<sf::Vector2f>(sf::Vector2u{windowWidth, windowHeight});

    boardView.setCenter(centerOfWindow);
    boardView.setSize(sizeOfWindow);

    uiView.setCenter(centerOfWindow);
    uiView.setSize(sizeOfWindow);

    historyView.setSize({ historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight });
    historyView.setCenter({ historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewHeight / 2.f });
    historyView.setViewport(renderer.getHistoryViewport());
}

void UI::resizeResignationButton() {
    resignationButton.position = { renderer.getWindowWidth() - boardMetrics.tileSize, renderer.getWindowHeight() - boardMetrics.offsetY};
    resignationButton.size = { boardMetrics.tileSize, boardMetrics.tileSize };
    resizeButtonBounds(resignationButton);
}

void UI::resizeResignationConfirmationLayout() {
    resignationConfirmationLayout.position = { historyViewportMetrics.historyViewStartX + historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewStartY + historyViewportMetrics.historyViewHeight / 2.f };
    resignationConfirmationLayout.size = { historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight };
}
void UI::resizeLayoutButtons() {
    resignationConfirmationLayout.cancelButton.size = { historyViewportMetrics.historyViewWidth / 4.f, historyViewportMetrics.historyViewWidth / 8.f };
    resignationConfirmationLayout.confirmButton.size = { historyViewportMetrics.historyViewWidth / 4.f, historyViewportMetrics.historyViewWidth / 8.f };

    const float xPositionBaseline = boardMetrics.offsetX + boardMetrics.boardSize + historyViewportMetrics.historyViewWidth / 2.f;
    const float yPositionBaseline = boardMetrics.offsetY + boardMetrics.boardSize / 2.f - boardMetrics.tileSize / 2.f;

    resignationConfirmationLayout.cancelButton.position = { xPositionBaseline - historyViewportMetrics.historyViewWidth / 4.f, yPositionBaseline + boardMetrics.tileSize };
    resignationConfirmationLayout.confirmButton.position = { xPositionBaseline + historyViewportMetrics.historyViewWidth / 4.f,  yPositionBaseline + boardMetrics.tileSize };

    resizeLayoutButtonBounds();
}

void UI::resizeLayoutButtonBounds() {
    resizeButtonBounds(resignationConfirmationLayout.cancelButton);
    resizeButtonBounds(resignationConfirmationLayout.confirmButton);
}

void UI::resizePromoLayout() {
    promoMenuLayout.panelPosition = { boardMetrics.boardRightEdge + renderer.getMargin(), boardMetrics.boardTopEdge};
    promoMenuLayout.panelSize = { boardMetrics.tileSize + (renderer.getPadding() * 2.f), (4.f * boardMetrics.tileSize) + (renderer.getPadding() * 2.f) + (3.f * renderer.getSpacing()) };

    for (std::size_t i = 0; i < promoMenuLayout.slotRects.size(); i++) {
        sf::FloatRect rect{};
        rect.position = { promoMenuLayout.panelPosition.x + renderer.getPadding(), promoMenuLayout.panelPosition.y + renderer.getPadding() + (static_cast<float>(i) * (boardMetrics.tileSize + renderer.getSpacing()))};
        rect.size = { boardMetrics.tileSize, boardMetrics.tileSize };
        promoMenuLayout.slotRects[i] = rect;
    }
}
void UI::recalculateMoveHistorySize() {
    renderer.setRowDistance(boardMetrics.tileSize / 2.f);
    renderer.setMoveHistorySize(game.getMoveHistory().size() * renderer.getRowDistance() + renderer.getMargin());
}

void UI::resizeMainMenuLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 3.f, boardMetrics.tileSize * 1.2f };
    mainMenuLayout.playButton.size = buttonSize;
    mainMenuLayout.settingsButton.size = buttonSize;
    mainMenuLayout.quitButton.size = buttonSize;
    
    const sf::Vector2f centerOfWindow = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() / 2.f };
    mainMenuLayout.playButton.position = centerOfWindow;
    mainMenuLayout.settingsButton.position = { centerOfWindow.x, centerOfWindow.y + buttonSize.y * 1.5f };
    mainMenuLayout.quitButton.position = { centerOfWindow.x, centerOfWindow.y + buttonSize.y * 3.f };

    resizeButtonBounds(mainMenuLayout.playButton);
    resizeButtonBounds(mainMenuLayout.settingsButton);
    resizeButtonBounds(mainMenuLayout.quitButton);
}

void UI::resizeButtonBounds(Button& button) {
    const float halfButtonWidth = button.size.x / 2.f;
    const float halfButtonHeight = button.size.y / 2.f;
    
    const sf::Vector2f boundsPosition = { button.position.x - halfButtonWidth, button.position.y - halfButtonHeight };
    const sf::Vector2f boundsSize = button.size;

    button.bounds = sf::FloatRect(boundsPosition, boundsSize);
}

void UI::resizePrePlayLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 1.25f, boardMetrics.tileSize * 1.25f};

    float currentXPosition = renderer.getWindowWidth() * 0.4f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;
    
    auto resizeSingleButton = [&](Button* button) {
        button->size = buttonSize;
        button->position = { currentXPosition, currentYPosition };
        resizeButtonBounds(*button);

        currentXPosition += buttonSize.x * 1.5f;
    };

    auto changePositions = [&]() {
        currentXPosition = renderer.getWindowWidth() * 0.4f + buttonSize.x / 2.f;
        currentYPosition += buttonSize.y * 1.5f;
    };

    for (Button* sideButton : prePlayLayout.sideButtons) {
        resizeSingleButton(sideButton);
    }
    changePositions();

    for (Button* opponentButton : prePlayLayout.opponentButtons) {
        resizeSingleButton(opponentButton);
    }
    changePositions();

    for (Button* timeControlButton : prePlayLayout.timeControlButtons) {
        resizeSingleButton(timeControlButton);
    }
    changePositions();

    for (Button* incrementButton : prePlayLayout.incrementButtons) {
        resizeSingleButton(incrementButton);
    }

    const sf::Vector2f startGameButtonSize = { boardMetrics.tileSize * 3.f, boardMetrics.tileSize * 1.25f };
    const sf::Vector2f startGameButtonPosition = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() - startGameButtonSize.y };
    prePlayLayout.startGameButton.size = startGameButtonSize;
    prePlayLayout.startGameButton.position = startGameButtonPosition;
    resizeButtonBounds(prePlayLayout.startGameButton);

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    prePlayLayout.backButton.size = backButtonSize;
    prePlayLayout.backButton.position = { renderer.getMargin() + backButtonSize.x / 2.f, renderer.getMargin() + backButtonSize.y / 2.f };
    resizeButtonBounds(prePlayLayout.backButton);
}

void UI::resizeSettingsLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 1.25f, boardMetrics.tileSize * 1.25f };
    float xPosition = renderer.getWindowWidth() / 2.f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;

    for (Button* toggleButton : settingsLayout.toggleButtons) {
        toggleButton->size = buttonSize;
        toggleButton->position = { xPosition, currentYPosition };
        resizeButtonBounds(*toggleButton);
        
        currentYPosition += buttonSize.y * 1.5f;
    }

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    settingsLayout.backButton.size = backButtonSize;
    settingsLayout.backButton.position = { renderer.getMargin() + backButtonSize.x / 2.f, renderer.getMargin() + backButtonSize.y / 2.f };
    resizeButtonBounds(settingsLayout.backButton);
}

void UI::resizeGameOverLayout() {
    const sf::Vector2f layoutSize = { renderer.getWindowWidth() * 0.4f, renderer.getWindowHeight() * 0.4f };
    gameOverLayout.size = layoutSize;

    const sf::Vector2f centerOfWindow = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() / 2.f };
    gameOverLayout.position = centerOfWindow;

    const sf::Vector2f buttonSize = { layoutSize.x * 0.3f, layoutSize.y * 0.25f };
    gameOverLayout.playAgainButton.size = buttonSize;
    gameOverLayout.mainMenuButton.size = buttonSize;

    gameOverLayout.playAgainButton.position = {
        centerOfWindow.x - (layoutSize.x * 0.25f),
        centerOfWindow.y + (layoutSize.y * 0.25f)
    };

    gameOverLayout.mainMenuButton.position = {
        centerOfWindow.x + (layoutSize.x * 0.25f),
        centerOfWindow.y + (layoutSize.y * 0.25f)
    };

    resizeButtonBounds(gameOverLayout.playAgainButton);
    resizeButtonBounds(gameOverLayout.mainMenuButton);
}