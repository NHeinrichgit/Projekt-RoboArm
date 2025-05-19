import time
import torch
from torch.utils.data import DataLoader
import torchvision.transforms as T
from model import get_model
from dataset import CocoDetectionDataset

def get_transform():
    return T.Compose([
        T.ToTensor(),
    ])

def collate_fn(batch):
    return tuple(zip(*batch))

def train_model():
    dataset = CocoDetectionDataset(
        root="dataset/images",
        annFile="dataset/annotations/_annotations.coco.json",
        transforms=get_transform(),
        return_masks=False,  # Set False if you're training Faster R-CNN
        cache_images=True
    )

    data_loader = DataLoader(dataset, batch_size=2, shuffle=True, collate_fn=collate_fn, num_workers = 8)

    device = torch.device("cuda") if torch.cuda.is_available() else torch.device("cpu")

    model = get_model(num_classes=4)
    model.to(device)

    # Optimizer
    params = [p for p in model.parameters() if p.requires_grad]
    optimizer = torch.optim.SGD(params, lr=0.005, momentum=0.9, weight_decay=0.0005)

    # Training loop
    num_epochs = 10
    imgcounter = 0
    model.train()
    for epoch in range(num_epochs):
        for images, targets in data_loader:
            start_time = time.time()
            images = list(img.to(device) for img in images)
            targets = [{k: v.to(device) for k, v in t.items()} for t in targets]

            loss_dict = model(images, targets)
            losses = sum(loss for loss in loss_dict.values())

            optimizer.zero_grad()
            losses.backward()
            optimizer.step()
            end_time = time.time()
            runtime=end_time-start_time
            imgcounter+=1
            print(f"progress logging {imgcounter}/2709: , time: {runtime:.2f} seconds")
        print(f"Epoch {epoch+1}, Loss: {losses.item():.4f}")
        torch.save(model.state_dict(), "fasterrcnn_cup_model.pth")
