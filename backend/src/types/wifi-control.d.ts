declare module 'wifi-control' {
  interface WiFiCredentials {
    ssid: string;
    password: string;
  }

  interface WiFiControl {
    init(config: { debug: boolean }): void;
    connectToAP(credentials: WiFiCredentials, callback: (err: Error | null, response: any) => void): void;
    getIfaceState(): any;
  }

  const wifiControl: WiFiControl;
  export default wifiControl;
} 