from flask import Flask, render_template, jsonify, request

app = Flask(__name__)

# Use better variable names and initialize state
device_state = {
    'fan': False,
    'light': False
}


@app.route('/activate',methods=['GET'])
def activate():
    print("Activate Request")
    return jsonify({'status': 'success'}), 200


@app.route('/get_status')
def get_status():
    return jsonify(device_state)


@app.route('/update_status', methods=['POST'])
def update_status():
    global device_state
    data = request.get_json()

    if data is None:
        return jsonify({'error': 'Invalid JSON'}), 400

    if 'fan' in data:
        device_state['fan'] = bool(data['fan'])
    if 'light' in data:
        device_state['light'] = bool(data['light'])

    return jsonify({'status': 'success'}), 200


@app.route('/')
def main():
    return render_template('index.html')


if __name__ == '__main__':
    app.run(host='0.0.0.0',debug=True)