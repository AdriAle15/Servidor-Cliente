import React from 'react';
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer } from 'recharts';
import { Device } from '../../types/device';

interface GraphWidgetProps {
  device: Device;
}

export const GraphWidget: React.FC<GraphWidgetProps> = ({ device }) => {
  const data = device.data?.values?.map((value, index) => ({
    name: `${index}m`,
    value,
  })) || [];

  return (
    <div className="h-48">
      <ResponsiveContainer width="100%" height="100%">
        <LineChart data={data}>
          <XAxis dataKey="name" />
          <YAxis />
          <Tooltip />
          <Line
            type="monotone"
            dataKey="value"
            stroke="#3b82f6"
            strokeWidth={2}
            dot={false}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};