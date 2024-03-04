from flask import Flask, request, make_response, jsonify
import hashlib
from datetime import datetime

app = Flask(__name__)

@app.route('/test')
def one():
    return "Flask has been connected"

@app.route('/resource/etag', methods=['GET'])
def check_etag():
    data = "something"
    etag = hashlib.sha1(data.encode('utf-8')).hexdigest()
    print(etag)
    if 'If-None-Match' in request.headers and request.headers['If-None-Match'] == etag:
        return '', 304
    response = make_response(jsonify({"data": data}))
    response.headers['Etag'] = etag
    response.headers['Cache-Control'] = 'no-cache'
    return response

@app.route('/resource/last_modified', methods=['GET'])
def check_last_modified():
    client_time = request.headers.get('If-Modified-Since')
    last_modified = datetime.now()  # This should be the actual last modification time of your resource
    # Convert last_modified and client_time to the same format for comparison
    if client_time and last_modified <= datetime.strptime(client_time, "%a, %d %b %Y %H:%M:%S GMT"):
        return '', 304
    response = make_response(jsonify({"data": "something"}))
    response.headers['Last-Modified'] = last_modified.strftime("%a, %d %b %Y %H:%M:%S GMT")
    return response

if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=5000)