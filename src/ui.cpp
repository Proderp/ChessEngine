#include "ui.hpp"

UI::UI(sf::RenderWindow& window, const sf::Vector2f& windowSize) :
    window(window),
    windowSize(windowSize)
{
    updateUI();

    boardView = window.getDefaultView();
    uiView = window.getDefaultView();
    historyView = window.getDefaultView();
    
    historyView.setViewport(historyViewport);
    historyView.setSize({ historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight });
    historyView.setCenter({ historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewHeight / 2.f });
}

void UI::updateUI() {
    recalibrateViews();
    
    resizeResignationButton();

    resizeResignationConfirmationLayout();

    resizeLayoutButtons();

    resizePromoLayout();
    recalculateMoveHistorySize();

    resizeMainMenuLayout();
    
    resizePrePlayLayout();
    resizeSettingsLayout();

    resizeGameOverLayout();

    setBoardSizes();
	setOffsets();
	setBoardEdges();
	setHistoryViewport();
}

void UI::recalibrateViews() {
    const sf::Vector2f centerOfWindow = {windowSize.x / 2.f, windowSize.y / 2.f};

    boardView.setCenter(centerOfWindow);
    boardView.setSize(windowSize);

    uiView.setCenter(centerOfWindow);
    uiView.setSize(windowSize);

    historyView.setSize({ historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight });
    historyView.setCenter({ historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewHeight / 2.f });
    historyView.setViewport(historyViewport);
}

void UI::resizeResignationButton() {
    resignationButton.position = { windowSize.x - boardMetrics.tileSize, windowSize.y - boardMetrics.offsetY};
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
    promoMenuLayout.panelPosition = { boardMetrics.boardRightEdge + uiConsts.margin, boardMetrics.boardTopEdge};
    promoMenuLayout.panelSize = { boardMetrics.tileSize + (uiConsts.padding * 2.f), (4.f * boardMetrics.tileSize) + (uiConsts.padding * 2.f) + (3.f * uiConsts.spacing) };

    for (std::size_t i = 0; i < promoMenuLayout.slotRects.size(); i++) {
        sf::FloatRect rect{};
        rect.position = { promoMenuLayout.panelPosition.x + uiConsts.padding, promoMenuLayout.panelPosition.y + uiConsts.padding + (static_cast<float>(i) * (boardMetrics.tileSize + uiConsts.spacing))};
        rect.size = { boardMetrics.tileSize, boardMetrics.tileSize };
        promoMenuLayout.slotRects[i] = rect;
    }
}

void UI::recalculateMoveHistorySize() {
    renderer.setRowDistance(boardMetrics.tileSize / 2.f);
    renderer.setMoveHistorySize(game.getMoveHistory().size() * renderer.getRowDistance() + uiConsts.margin);
}

void UI::resizeMainMenuLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 3.f, boardMetrics.tileSize * 1.2f };
    mainMenuLayout.playButton.size = buttonSize;
    mainMenuLayout.settingsButton.size = buttonSize;
    mainMenuLayout.quitButton.size = buttonSize;
    
    const sf::Vector2f centerOfWindow = { windowSize.x / 2.f, windowSize.y / 2.f };
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

    float currentXPosition = windowSize.x * 0.4f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;
    
    auto resizeSingleButton = [&](Button* button) {
        button->size = buttonSize;
        button->position = { currentXPosition, currentYPosition };
        resizeButtonBounds(*button);

        currentXPosition += buttonSize.x * 1.5f;
    };

    auto changePositions = [&]() {
        currentXPosition = windowSize.x * 0.4f + buttonSize.x / 2.f;
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
    const sf::Vector2f startGameButtonPosition = { windowSize.x / 2.f, windowSize.y - startGameButtonSize.y };
    prePlayLayout.startGameButton.size = startGameButtonSize;
    prePlayLayout.startGameButton.position = startGameButtonPosition;
    resizeButtonBounds(prePlayLayout.startGameButton);

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    prePlayLayout.backButton.size = backButtonSize;
    prePlayLayout.backButton.position = { uiConsts.margin + backButtonSize.x / 2.f, uiConsts.margin + backButtonSize.y / 2.f };
    resizeButtonBounds(prePlayLayout.backButton);
}

void UI::resizeSettingsLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 1.25f, boardMetrics.tileSize * 1.25f };
    float xPosition = windowSize.x / 2.f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;

    for (Button* toggleButton : settingsLayout.toggleButtons) {
        toggleButton->size = buttonSize;
        toggleButton->position = { xPosition, currentYPosition };
        resizeButtonBounds(*toggleButton);
        
        currentYPosition += buttonSize.y * 1.5f;
    }

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    settingsLayout.backButton.size = backButtonSize;
    settingsLayout.backButton.position = { uiConsts.margin + backButtonSize.x / 2.f, uiConsts.margin + backButtonSize.y / 2.f };
    resizeButtonBounds(settingsLayout.backButton);
}

void UI::resizeGameOverLayout() {
    const sf::Vector2f layoutSize = { windowSize.x * 0.4f, windowSize.y * 0.4f };
    gameOverLayout.size = layoutSize;

    const sf::Vector2f centerOfWindow = { windowSize.x / 2.f, windowSize.y / 2.f };
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

void UI::setBoardSizes() {
	float smallerDimension = std::min(windowSize.x, windowSize.y);

	boardMetrics.boardSize = smallerDimension * 0.8f;
	boardMetrics.tileSize = boardMetrics.boardSize / 8.f;
	boardMetrics.tileSize = boardMetrics.tileSize;

	boardMetrics.circleSize = boardMetrics.tileSize / 8.f;
}
void UI::setOffsets() {
	boardMetrics.offsetX = boardMetrics.tileSize * 1.5f;
	boardMetrics.offsetY = ((windowSize.y - boardMetrics.boardSize) / 2.f) + (boardMetrics.tileSize / 2.f);
}
void UI::setBoardEdges() {
	boardMetrics.boardRightEdge = boardMetrics.offsetX + ((FILES - 1) * boardMetrics.tileSize) + (boardMetrics.tileSize / 2.f);
	boardMetrics.boardTopEdge = boardMetrics.offsetY - (boardMetrics.tileSize / 2.f);
}
void UI::setHistoryViewport() {
	historyViewportMetrics.historyViewStartX = boardMetrics.offsetX + boardMetrics.boardSize;
	historyViewportMetrics.historyViewStartY = boardMetrics.offsetY + boardMetrics.tileSize;

	historyViewportMetrics.historyViewWidth = windowSize.x - historyViewportMetrics.historyViewStartX - boardMetrics.tileSize / 2.f;
	historyViewportMetrics.historyViewHeight = windowSize.y - historyViewportMetrics.historyViewStartY * 2;

	historyViewportMetrics.viewportXPercentage = historyViewportMetrics.historyViewStartX / windowSize.x;
	historyViewportMetrics.viewportYPercentage = historyViewportMetrics.historyViewStartY / windowSize.y;

	historyViewportMetrics.viewportWidthPercentage = historyViewportMetrics.historyViewWidth / windowSize.x;
	historyViewportMetrics.viewportHeightPercentage = historyViewportMetrics.historyViewHeight / windowSize.y;

	sf::Vector2f viewPosition{ historyViewportMetrics.viewportXPercentage, historyViewportMetrics.viewportYPercentage };
	sf::Vector2f viewSize{ historyViewportMetrics.viewportWidthPercentage, historyViewportMetrics.viewportHeightPercentage };

	historyViewport = sf::FloatRect(viewPosition, viewSize);
}

const float UI::getMargin() const {
    return uiConsts.margin;
}

void UI::setUIView() {
    window.setView(uiView);
}

void UI::setHistoryView() {
    window.setView(historyView);
}

void UI::setBoardView() {
    window.setView(boardView);
}

const sf::View& UI::getUIView() {
    return uiView;
}

const sf::View& UI::getHistoryView() {
    return historyView;
}

const sf::View& UI::getBoardView() {
    return boardView;
}

void UI::moveHistoryView(const sf::Vector2f position) {
    historyView.move(position);
}

void UI::setHistoryViewCenter(const sf::Vector2f center) {
    historyView.setCenter(center);
}

const Button& UI::getResignationButton() {
    return resignationButton;
}

const PromoMenuLayout& UI::getPromoMenuLayout() {
    return promoMenuLayout;
}

const ResignationConfirmationLayout& UI::getResignationConfirmationLayout() {
    return resignationConfirmationLayout;
}

const MainMenuLayout& UI::getMainMenuLayout() {
    return mainMenuLayout; 
}

const PrePlayLayout& UI::getPrePlayLayout() {
    return prePlayLayout;
}

const SettingsLayout& UI::getSettingsLayout() {
    return settingsLayout; 
}

const GameOverLayout& UI::getGameOverLayout() {
    return gameOverLayout;
}

const HistoryViewportMetrics& UI::getHistoryViewportMetrics() {
    return historyViewportMetrics;
}

const BoardMetrics& UI::getBoardMetrics() {
    return boardMetrics; 
}