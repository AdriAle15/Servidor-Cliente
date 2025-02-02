export type DeviceType = 'button';
export type WidgetType = 'button';

export interface Device {
  id: string;
  name?: string;
  identifier?: string;
  type?: DeviceType;
  widgetType?: WidgetType;
  ip: string;
  status: 'unconfigured' | 'configured' | 'error';
  data?: {
    state?: 'on' | 'off';
  };
}