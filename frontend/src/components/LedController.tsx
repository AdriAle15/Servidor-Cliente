import React, { useState, useEffect } from 'react';
import { wsService } from '../services/websocket.service';

interface LedState {
  led1: 'on' | 'off';
  led2: 'on' | 'off';
  led3: 'on' | 'off';
}

export const LedController: React.FC = () => {
  const [ledState, setLedState] = useState<LedState>({
    led1: 'off',
    led2: 'off',
    led3: 'off'
  });

  useEffect(() => {
    const handleMessage = (data: any) => {
      if (data.type === 'device_update' && data.data) {
        setLedState(data.data);
      }
    };

    wsService.addMessageHandler(handleMessage);
  }, []);

  const toggleLed = (ledId: 'led1' | 'led2' | 'led3') => {
    const newState = ledState[ledId] === 'on' ? 'off' : 'on';
    wsService.sendMessage({
      type: 'led_control',
      ledId,
      state: newState
    });
  };

  return (
    <div className="led-controller">
      <h2>Control de LEDs</h2>
      <div className="led-buttons">
        {(['led1', 'led2', 'led3'] as const).map((ledId) => (
          <button
            key={ledId}
            className={`led-button ${ledState[ledId]}`}
            onClick={() => toggleLed(ledId)}
          >
            {ledId.toUpperCase()} - {ledState[ledId].toUpperCase()}
          </button>
        ))}
      </div>
    </div>
  );
}; 