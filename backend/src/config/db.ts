import { Pool } from 'pg';

export const pool = new Pool({
  user: 'postgres',
  host: 'localhost',
  database: 'cliente',
  password: 'sosamejia',
  port: 5432,
}); 