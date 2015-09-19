class Api::V1::ImportsController < Api::V1::ApiController
  # respond_to :json
  skip_before_filter :verify_authenticity_token

  def post_tempature
    Rails.logger.info "*** API *** post_tempature"
    Rails.logger.info "*** => params:#{params}"
    # if params[:article_id] && params[:article_id].to_i != 0
    # end
    # render json: {message: "article_import Hit"}
    render json: {
      status: 200,
      message: "post_tempature Hit",
    }.to_json
  end

  def get_tempature
    render :json => {message: "get_tempature Get"}
  end
end

# $.ajax({
  # 	url: "http://dgo.bdmedia.com/api/import/article",
# 	cache: false,
# 	dataType: 'json',
# 	type: "POST",
#   headers: {
#     'Token':  '84jd90dk30dktov41kfmct3bd94md9c6c43x3su34n42',
#     'Accept': 'v1'
#   },
# 	data: {
#     article_id: "42"
#   }
# });

