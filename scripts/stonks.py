import warnings

warnings.filterwarnings("ignore", category=RuntimeWarning)
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)
import requests
import pandas as pd
import json
import plyvel
import time

# How long to sleep between each request
sleep_time = 0.9

# --------------- GLOBAL PREFERENCES ---------------------- #
API_KEY = '6c95f1047dmsh33044bdcb64641fp1ccbc0jsnf78737aae9d4'
# Boolean switch so that you only run historical data once or on-demand
historical_data = False
nasdaq_csv = False
company_overview = False
refresh_history = False
refresh_daily = True
# --------------------------------------------------------- #

csv_location = 'd:\\github\\stonks\\scripts\\tickers.csv'
csv_refresh_location = 'd:\\github\\stonks\\scripts\\refresh.csv'
db_dir = 'd:\\github\\stonks\\stock-market'

# Bump this to force a refresh of all historical data
have_price = 'have_price1' 
no_price = 'no_price'


def upload_csv(csv_file):
    if nasdaq_csv:
        df = pd.read_csv(csv_file, header=0)
    else:
        df = pd.read_csv(csv_file)

    sorted_df = df.iloc[:, 0].tolist()
    return sorted_df


# gets entire available data history at AV
def get_historical_quotes(tickers_csv_file, db):
    for ticker in tickers_csv_file:

        stock_id = 'price' + '.' + ticker;
        stock_key = bytes(stock_id, encoding='utf-8')
        if db.get(stock_key):
            msg = db.get(stock_key)
            print(f'Skipping stock {ticker} data already found {msg}.')
        else:
            time.sleep(sleep_time)

            url = "https://alpha-vantage.p.rapidapi.com/query"

            querystring = {"function": "TIME_SERIES_DAILY_ADJUSTED", "symbol": ticker, "datatype": "json",
                        "outputsize": "full"}

            headers = {
                'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
                'x-rapidapi-key': f"{API_KEY}"
            }
            #print(f'Requesting historical price data for {ticker}')
            response = requests.request("GET", url, headers=headers, params=querystring)
            data = response.json()

            try:
                stock_keys_list = list(data['Time Series (Daily)'].keys())
                print(f'Saving historical price data for {ticker}')
                db.put(stock_key,bytes(have_price,encoding='utf-8'))
                for date_key in stock_keys_list:
                    stock_date = date_key
                    stock_open_price = str(data['Time Series (Daily)'][date_key]['1. open'])
                    stock_identifier = 'price' + '.' + ticker + '.' + stock_date
                    #print(f'Key({stock_identifier}) Value({stock_open_price})')
                    bkey = bytes(stock_identifier, encoding='utf-8')
                    bvalue = bytes(stock_open_price, encoding='utf-8')
                    db.put(bkey, bvalue)
            except Exception as e:
                db.put(stock_key,bytes(no_price,encoding='utf-8'))
                jsonstring = json.dumps(data)
                print(f'Error({jsonstring}) accessing the time series for ticker {ticker}. Skipping it.')



# gets entire available data history at AV
def refresh_historical_quotes(tickers_csv_file, db):
    for ticker in tickers_csv_file:

        stock_id = 'price' + '.' + ticker;
        stock_key = bytes(stock_id, encoding='utf-8')
        time.sleep(sleep_time)

        url = "https://alpha-vantage.p.rapidapi.com/query"

        querystring = {"function": "TIME_SERIES_DAILY_ADJUSTED", "symbol": ticker, "datatype": "json",
                    "outputsize": "full"}

        headers = {
            'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
            'x-rapidapi-key': f"{API_KEY}"
        }
        #print(f'Requesting historical price data for {ticker}')
        response = requests.request("GET", url, headers=headers, params=querystring)
        data = response.json()

        try:
            stock_keys_list = list(data['Time Series (Daily)'].keys())
            print(f'Saving historical price data for {ticker}')
            db.put(stock_key,bytes(have_price,encoding='utf-8'))
            for date_key in stock_keys_list:
                stock_date = date_key
                stock_open_price = str(data['Time Series (Daily)'][date_key]['1. open'])
                stock_identifier = 'price' + '.' + ticker + '.' + stock_date
                #print(f'Key({stock_identifier}) Value({stock_open_price})')
                bkey = bytes(stock_identifier, encoding='utf-8')
                bvalue = bytes(stock_open_price, encoding='utf-8')
                db.put(bkey, bvalue)
        except Exception as e:
            db.put(stock_key,bytes(no_price,encoding='utf-8'))
            jsonstring = json.dumps(data)
            print(f'Error({jsonstring}) accessing the time series for ticker {ticker}. Skipping it.')


# gets entire available data history at AV
def refresh_daily_quotes(tickers_csv_file, db):
    for ticker in tickers_csv_file:

        stock_id = 'price' + '.' + ticker;
        stock_key = bytes(stock_id, encoding='utf-8')
        time.sleep(sleep_time)

        url = "https://alpha-vantage.p.rapidapi.com/query"

        querystring = {"function": "GLOBAL_QUOTE", "symbol": ticker, "datatype": "json"}

        headers = {
            'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
            'x-rapidapi-key': f"{API_KEY}"
        }
        #print(f'Requesting historical price data for {ticker}')
        response = requests.request("GET", url, headers=headers, params=querystring)
        data = response.json()
        #print(data)

        try:
            stock_open_price = data['Global Quote']['05. price']
            stock_date = data['Global Quote']['07. latest trading day']

            print(f'[{ticker}] : Price:{stock_open_price} Date:{stock_date}')

            stock_identifier = 'price' + '.' + ticker + '.' + stock_date
            bkey = bytes(stock_identifier, encoding='utf-8')
            bvalue = bytes(stock_open_price, encoding='utf-8')
            db.put(bkey, bvalue)
        except Exception as e:
            jsonstring = json.dumps(data)
            print(f'Error({jsonstring}) Failed to get price for ticker {ticker}. Skipping it.')


def get_fundamentals(tickers_csv_file, db):
    for ticker in tickers_csv_file:
        ticker_symbol = 'ticker.' + ticker;
        bkey = bytes(ticker_symbol, encoding='utf-8')
        if db.get(bkey):
            print(f'Skipping {ticker} we already have data for it')
        else:
            time.sleep(sleep_time)
            url = "https://alpha-vantage.p.rapidapi.com/query"

            querystring = {"function": "OVERVIEW", "symbol": ticker, "datatype": "json",
                        "output_size": "compact"}

            headers = {
                'x-rapidapi-host': "alpha-vantage.p.rapidapi.com",
                'x-rapidapi-key': f"{API_KEY}"
            }
            response = requests.request("GET", url, headers=headers, params=querystring)
            data = response.json()
            try:
                symbol = data['Symbol']
                ticker_symbol = 'ticker.' + symbol
                bkey = bytes(ticker_symbol, encoding='utf-8')
                jsonstring = json.dumps(data)
                bvalue = bytes(jsonstring, encoding='utf-8')
                db.put(bkey,bvalue)
                print(f'Saving fundamentals data for ticker:{ticker}')
            except Exception as e:
                db.put(bkey,bytes(no_price,encoding='utf-8'))
                jsonstring = json.dumps(data)
                print(f'Error({jsonstring}) accessing the fundamentals for ticker {ticker}. Skipping it.')


def test():
    url = "https://twelve-data1.p.rapidapi.com/price"

    querystring = {"symbol":"AAPL","format":"json","outputsize":"30"}

    headers = {
        'x-rapidapi-host': "twelve-data1.p.rapidapi.com",
        'x-rapidapi-key': "6c95f1047dmsh33044bdcb64641fp1ccbc0jsnf78737aae9d4"
        }

    response = requests.request("GET", url, headers=headers, params=querystring)

    print(response.text)


if __name__ == "__main__":
    # daily run
    test()

    db = plyvel.DB(db_dir, create_if_missing=True)

    refresh_csv = upload_csv(csv_refresh_location)

    if refresh_history:
        refresh_historical_quotes(refresh_csv, db)

    if refresh_daily:
        refresh_daily_quotes(refresh_csv, db)

    sorted_df_sliced = upload_csv(csv_location)

    # historical run for the entire price history
    if historical_data:
        get_historical_quotes(sorted_df_sliced, db)

    if company_overview:
        get_fundamentals(sorted_df_sliced, db)

    db.close()

