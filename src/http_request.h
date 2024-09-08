#pragma once

/**
  Could put all this logic in the request class but that would get messy.
  Each request will send one http_request and receive one http_response.
  It cout be a single http_request built by the main thread and used by all the request objects. 
class http_request
{

};
