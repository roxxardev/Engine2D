#include "ScreenList.h"

#include "IGameScreen.h"

namespace Engine{

	ScreenList::ScreenList(IMainGame* game) :
		m_game(game) {
	}

	ScreenList::~ScreenList()
	{
		destroy();
	}

	IGameScreen* Engine::ScreenList::moveNext()
	{
		IGameScreen* currentScreen = getCurrent();
		if (currentScreen->getNextScreenIndex() != SCREEN_INDEX_NO_SCREEN) {
			m_curentScreenIndex = currentScreen->getNextScreenIndex();
		}
		return getCurrent();
	}

	IGameScreen* Engine::ScreenList::movePrevious()
	{
		IGameScreen* currentScreen = getCurrent();
		if (currentScreen->getPreviousScreenIndex() != SCREEN_INDEX_NO_SCREEN) {
			m_curentScreenIndex = currentScreen->getPreviousScreenIndex();
		}
		return getCurrent();
	}

	void Engine::ScreenList::setScreen(int nextScreen)
	{
		m_curentScreenIndex = nextScreen;
	}

	void Engine::ScreenList::addScreen(IGameScreen* newScreen)
	{
		newScreen->m_screenIndex = m_screens.size();
		m_screens.push_back(newScreen);
		newScreen->build();
		newScreen->setParentGame(m_game);
	}

	void Engine::ScreenList::destroy()
	{
		for (size_t i = 0; i < m_screens.size(); i++) {
			m_screens[i]->destroy();
		}
		m_screens.resize(0);
		m_curentScreenIndex = SCREEN_INDEX_NO_SCREEN;
	}

	IGameScreen* ScreenList::getCurrent()
	{
		if (m_curentScreenIndex == SCREEN_INDEX_NO_SCREEN) return nullptr;
		return m_screens[m_curentScreenIndex];
	}

}