CREATE TABLE IF NOT EXISTS dispositivos_conectados (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255),
    type VARCHAR(50),
    ip VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL,
    data JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

DROP TRIGGER IF EXISTS update_dispositivos_conectados_updated_at ON dispositivos_conectados;
CREATE TRIGGER update_dispositivos_conectados_updated_at
    BEFORE UPDATE ON dispositivos_conectados
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column(); 