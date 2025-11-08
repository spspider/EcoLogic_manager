-- Add conditions column to devices table
ALTER TABLE devices 
ADD COLUMN conditions JSON DEFAULT NULL
COMMENT 'JSON array of automation conditions';
