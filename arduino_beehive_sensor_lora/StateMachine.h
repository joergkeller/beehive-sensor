/**********************************************************
 * Handling the state and notifications on state changes.
 * ---
 * - handling of user-defined states with names
 * - handle entry and exit of states
 * - handle state callback on loop
 * - handle state timeout
 * Client code moves direct state transitions, there are no
 * application-level events that map to specific transitions.
 * All potential state transitions are allowed.
 **********************************************************/
#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#define INVALID_STATE -1
#define INVALID_DURATION 0

typedef void (*StateHandler)();
typedef unsigned long (*TimeFunction)();
typedef struct {
  unsigned long maxDuration;
  StateHandler  handler;
} TimeoutHandler;

class StateMachine {
  public:
    StateMachine(int stateCount, const char** names, TimeFunction millis) 
    : stateNames(names), 
      timeFunction(millis),
      enterHandler(new StateHandler[stateCount]), 
      stateHandler(new StateHandler[stateCount]), 
      timeoutHandler(new TimeoutHandler[stateCount]), 
      exitHandler(new StateHandler[stateCount]) {
      TimeoutHandler noTimeout = {INVALID_DURATION, 0};
      for (int i = 0; i < stateCount; i++) {
        enterHandler[i] = 0;
        stateHandler[i] = 0;
        timeoutHandler[i] = noTimeout;
        exitHandler[i] = 0;
      }
    }

    void toState(int state) {
      if (nextState != INVALID_STATE) {
        Serial.print("Error! Not processed state transition to ");
        Serial.println(stateName(nextState));
      }
      nextState = state;
    }

    inline
    int state() { return currentState; }

    inline
    const char* stateName(int state) {
      return stateNames[state];
    }

    inline
    unsigned long duration() {
      return timeFunction() - startTime;
    }

    void onEnter(int state, StateHandler handler) {
      enterHandler[state] = handler;
    }

    void onState(int state, StateHandler handler) {
      stateHandler[state] = handler;
    }

    void onTimeout(int state, unsigned long maxDuration, StateHandler handler) {
      TimeoutHandler timeout = {maxDuration, handler};
      timeoutHandler[state] = timeout;
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
        if (timeoutHandler[currentState].maxDuration != INVALID_DURATION && timeoutHandler[currentState].maxDuration < duration()) {
          onTimeoutState(currentState);
        }
      }
    }

  protected:
    void changeState(int nextState) {
      if (currentState != INVALID_STATE) {
        onExitState(currentState);
      }
      currentState = nextState;
      startTime = timeFunction();
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
    
    void onTimeoutState(int state) {
      Serial.print("Timeout state "); Serial.println(stateName(state));
      if (timeoutHandler[state].handler != 0) {
        timeoutHandler[state].handler();
      }
    }
    
    void onExitState(int oldState) {
      if (exitHandler[oldState] != 0) {
        exitHandler[oldState]();
      }
      Serial.print("Exit state "); Serial.println(stateName(oldState));
    }

  private:
    unsigned long startTime;
    int currentState = INVALID_STATE;
    volatile int nextState = INVALID_STATE;
    const char** stateNames;
    TimeFunction timeFunction;
    StateHandler* enterHandler;
    StateHandler* stateHandler;
    TimeoutHandler* timeoutHandler;
    StateHandler* exitHandler;
};

#endif
