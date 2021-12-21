import pandas as pd
import glob
files = glob.glob('stock_*.csv')
df = pd.DataFrame()
for file in files:
    df_aux = pd.read_csv(file)
    df_aux['stock'] = file[6:]
    df = pd.concat([df, df_aux])
df['Date'] = pd.to_datetime(df['Date'])
df['stock'] = df['stock'].apply(lambda x: x[0:4])
df.reset_index(drop=True, inplace=True)
data = df[['stock', 'Adj Close', 'Date']].pivot(index='Date', columns='stock', values='Adj Close')
#data.fillna(0, inplace=True)
data.dropna(inplace=True)
data.to_csv('stocks_data_procesed.csv', index=True)