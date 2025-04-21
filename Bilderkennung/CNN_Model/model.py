import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor

# this function generates a premade model which comes with torch.
# in our case, num_classes is two, one class is the cup (region of interest), one class is the background (everything that isnt interesting)
def get_model(num_classes):
    # Load pre-trained model (model already "knows how to see")
    model = torchvision.models.detection.fasterrcnn_resnet50_fpn(pretrained=True)
    
    # grab the number of inputs going into the final layer of the premade model
    in_features = model.roi_heads.box_predictor.cls_score.in_features
    # replace final layer with our own using the SAME number of inputs as before and our number of classes to classify
    model.roi_heads.box_predictor = FastRCNNPredictor(in_features, num_classes)
    return model