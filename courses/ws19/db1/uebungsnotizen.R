# Übung 10 Datenbanksysteme I: Grafische Datenanalyse

#install.packages("ggplot2") and other below

library(ggplot2)
library(sqldf)

#FIXME
folder='....Pfad zu den CSV-Dateien....'

# lese csv Dateien
dsym = read.csv(paste(folder, 'sym.csv', sep='/'))
dhist = read.csv(paste(folder, 'hist.csv', sep='/'))


# beisipiel selects in R
dsym[10:20,]
dsym[dsym$sharesoutstanding > 10000,]

# aber wir können doch SQL:
countries = sqldf("select country, count(*) as cnt
        from dsym
        group by country
        order by cnt desc
        limit 10")

# ggplot mit layer
ggplot(countries) +
  layer(geom='bar', stat="identity", 
        position="identity", mapping=aes(x=reorder(country, -cnt), y=cnt))

# Alternative kürzer:
ggplot(countries) +
  geom_bar(aes(x=reorder(country, -cnt), y=cnt), stat='identity')

sqldf("select ticker, MAX(day)
        from dhist
        group by ticker")


# Finde den letzten Preis für jeden Ticker (latestDay)
# Multipliziere diesen mit sharesOutstanding
marketCap = sqldf("
        with latestDay as (
          select ticker, MAX(day) maxDay
                  from dhist
                  group by ticker
        )
        select s.*, close, sharesOutstanding*close as cap
          from dsym s 
            join dhist h on s.ticker = h.ticker
            join latestDay lp on s.ticker = lp.ticker and h.day = lp.maxDay
            order by cap desc
            limit 10
      ")
head(marketCap)

# Bereinige fehlerhafte Daten mit update-Befehl (Im neuen Datensatz bereits geändert)
#dhist = sqldf(c("update dhist set close = close / 100 where ticker = 'HON.L'" , "select * from main.dhist"))
head(sqldf("select * from dhist where ticker = 'HON.L' order by close desc"))

# Market Cap plot
ggplot(marketCap) + 
  geom_bar(mapping=aes(x=reorder(ticker, cap), y=cap/1e9), stat='identity') +
  coord_flip() +
  xlab('company') + 
  ylab('market cap [B$]') +
  ggtitle("Market Cap") +
  theme_bw()

# Aktienkurse der größten Unternehmen
dhistFil = sqldf("
          with latestDay as (
            select ticker, MAX(day) maxDay
                    from dhist
                    group by ticker
          ),
          lastPrice as (
            select * 
                from dhist sh join latestDay as ld on ld.ticker = sh.ticker and sh.day = ld.maxDay
          ),
          --- calculate market cap (like above)
          marketCap as (
            select s.name, s.c_name, s.ticker,
            (s.name || ' ' || s.ticker) tickerName,
            s.sharesOutstanding, lp.close,
            s.sharesOutstanding*lp.close as cap 
            from dsym s join lastPrice lp on lp.ticker = s.ticker
          )
          --- get the historic share prices for the 5 highest valued companies (using marketCap)
          select *
					from dsym s 
						join dhist sh on s.ticker = sh.ticker
					where s.ticker in (select ticker from marketCap order by cap desc limit 5)
                --and sh.day > date('2019-01-01')")
dhistFil$day = as.Date(dhistFil$day)
head(dhistFil)

ggplot(data=dhistFil) +
  geom_line(aes(x=day, y=close, color=ticker)) + 
  #xlim(c(as.Date('2019-01-01'), as.Date('2019-06-30'))) +
  ggtitle("Historic Price Chart for ") + 
  ylab("price [$]") +
  xlab("time [$]") +
  #expand_limits(y = 0) +
  facet_grid(c_name ~ ., scales="free") +
  #facet_wrap(industry ~ ., scales="free") +
  theme_bw()

# weitere Beispiele:
dhistFil = sqldf("select *
  from dsym s 
      join dhist sh on s.ticker = sh.ticker
  where s.ticker like 'GOOG%'")
dhistFil$day = as.Date(dhistFil$day)

ggplot(data=dhistFil) +
  geom_point(aes(x=day, y=close, color=ticker), size=0.7, alpha=0.5) + 
  #xlim(c(as.Date('2019-01-01'), as.Date('2019-06-30'))) +
  theme_bw()



