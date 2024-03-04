FROM python:3.10-alpine
WORKDIR /test
COPY requirements.txt /test
RUN pip3 install -r requirements.txt
COPY . /test
ENTRYPOINT ["python3"]
CMD ["app.py"]
EXPOSE 5000
