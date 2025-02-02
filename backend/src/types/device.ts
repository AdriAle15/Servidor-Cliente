export interface Device {
  id: string;
  name?: string;
  type?: string;
  ip: string;
  status: string;
  data?: {
    state?: 'on' | 'off';
  };
  created_at?: Date;
  updated_at?: Date;
} 