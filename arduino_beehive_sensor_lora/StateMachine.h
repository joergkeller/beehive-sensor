/**********************************************************
 * Handling the state and notifications on state changes.
 * ---
 * - handling of user-defined states with names
 * - handle entry and exit of states
 **********************************************************/
#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#define INVALID_STATE -1

typedef void (*StateHandler)();

class StateMachine {
  public:
    StateMachine(int stateCount, const char** names) 
    : stateNames(names), 
      enterHandler(new StateHandler[stateCount]), 
      stateHandler(new StateHandler[stateCount]), 
      exitHandler(new StateHandler[stateCount]) {
      for (int i = 0; i < stateCount; i++) {
        enterHandler[i] = 0;
        stateHandler[i] = 0;
        exitHandler[i] = 0;
      }
    }

    void toState(int state) {
      if (nextState != INVALID_STATE) {
        Serial.println("Error! Not processed state transition.");
      }
      nextState = state;
    }

    inline
    int state() { return currentState; }

    inline
    const char* stateName(int state) {
      return stateNames[state];
    }

    void onEnter(int state, StateHandler handler) {
      enterHandler[state] = handler;
    }

    void onState(int state, StateHandler handler) {
      stateHandler[state] = handler;
    }

    void onExit(int state, StateHandler handler) {
      exitHandler[state] = handler;
    }

    void loop() {
      if (nextState != INVALID_STATE) {
        changeState(nextState);
        nextState = INVALID_STATE;
      }
      if (currentState != INVALID_STATE) {
        onLoopState(currentState);
      }
    }

  protected:
    void changeState(int nextState) {
      if (currentState != INVALID_STATE) {
        onExitState(currentState);
      }
      currentState = nextState;
      if (currentState != INVALID_STATE) {
        onEnterState(currentState);
      }
    }
  
    void onEnterState(int newState) {
      Serial.print("\nEnter state "); Serial.println(stateName(newState));
      if (enterHandler[newState] != 0) {
        enterHandler[newState]();
      }
    }
    
    void onLoopState(int state) {
      if (stateHandler[state] != 0) {
        stateHandler[state]();
      }
    }
    
    void onExitState(int oldState) {
      if (exitHandler[oldState] != 0) {
        exitHandler[oldState]();
      }
      Serial.print("Exit state "); Serial.println(stateName(oldState));
    }

  private:
    int currentState = INVALID_STATE;
    int nextState = INVALID_STATE;
    const char** stateNames;
    StateHandler* enterHandler;
    StateHandler* stateHandler;
    StateHandler* exitHandler;
};

#endif
