DROP TABLE IF EXISTS ZehnkampfD;

CREATE TABLE ZehnkampfD (
    Name VARCHAR(100) NOT NULL,
    Disziplin VARCHAR(100) NOT NULL,
    punkte INTEGER NOT NULL
);

insert into ZehnkampfD values
    ('Bolt',        '100m',         50),
    ('Bolt',        'Weitsprung',   50),
    ('Eaton',       '100m',         40),
    ('Eaton',       'Weitsprung',   60),
    ('Suarez',      '100m',         60),
    ('Suarez',      'Weitsprung',   60),
    ('Behrenbruch', '100m',         30),
    ('Behrenbruch', 'Weitsprung',   50)
;