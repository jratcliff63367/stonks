import warnings

warnings.filterwarnings("ignore", category=RuntimeWarning)
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)
import requests
import pandas as pd
import json
import plyvel

# --------------- GLOBAL PREFERENCES ---------------------- #
API_KEY = '6c95f1047dmsh33044bdcb64641fp1ccbc0jsnf78737aae9d4'
# Boolean switch so that you only run historical data once or on-demand
historical_data = True
nasdaq_csv = False
company_overview = True
# --------------------------------------------------------- #

csv_location = 'd:\\tickers.csv'
db_dir = 'd:\\github\\stonks\\stock-market'


def upload_csv(csv_file):
    if nasdaq_csv:
        df = pd.read_csv(csv_file, header=0)
    else:
        df = pd.read_csv(csv_file)

    sorted_df = df.iloc[:, 0].tolist()
    return sorted_df


# gets entire available data history at AV
def get_historical_quotes(tickers_csv_file):
    price_data = []
    for ticker in tickers_csv_file:
        url = "https://alpha-vantage.p.rapidapi.com/query"

        querystring = {"function": "TIME_SERIES_DAILY_ADJUSTED", "symbol": ticker, "datatype": "json",
                       "outputsize": "full"}

        headers = {
            'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
            'x-rapidapi-key': f"{API_KEY}"
        }
        print(f'Requesting historical price data for {ticker}')
        response = requests.request("GET", url, headers=headers, params=querystring)
        data = response.json()
        price_data.append(data)
    return price_data


def get_fundamentals(tickers_csv_file, db):
    for ticker in tickers_csv_file:
        url = "https://alpha-vantage.p.rapidapi.com/query"

        querystring = {"function": "OVERVIEW", "symbol": ticker, "datatype": "json",
                       "output_size": "compact"}

        headers = {
            'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
            'x-rapidapi-key': f"{API_KEY}"
        }

        response = requests.request("GET", url, headers=headers, params=querystring)
        data = response.json()
        symbol = data['Symbol']
        ticker_symbol = 'ticker.' + symbol
        bkey = bytes(ticker_symbol, encoding='utf-8')
        jsonstring = json.dumps(data)
        bvalue = bytes(jsonstring, encoding='utf-8')
        db.put(bkey,bvalue)


def organize_data_historical(tickers_csv_file,db):
    historical_quote_data = get_historical_quotes(tickers_csv_file)
    for stock in historical_quote_data:
        stock_symbol = stock['Meta Data']['2. Symbol']
        stock_keys_list = list(stock['Time Series (Daily)'].keys())
        for date_key in stock_keys_list:
            stock_date = date_key
            stock_open_price = str(stock['Time Series (Daily)'][date_key]['1. open'])
            stock_identifier = 'price' + '.' + stock_symbol + '.' + stock_date
            print(f'Key({stock_identifier}) Value({stock_open_price})')
            bkey = bytes(stock_identifier, encoding='utf-8')
            bvalue = bytes(stock_open_price, encoding='utf-8')
            db.put(bkey,bvalue)



if __name__ == "__main__":
    # daily run

    db = plyvel.DB(db_dir, create_if_missing=True)

    sorted_df_sliced = upload_csv(csv_location)

    # historical run for the entire price history
    if historical_data:
        organize_data_historical(sorted_df_sliced,db)

    if company_overview:
        get_fundamentals(sorted_df_sliced, db)

    db.close()

